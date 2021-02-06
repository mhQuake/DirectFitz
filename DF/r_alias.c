/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

//r_alias.c -- alias model rendering

#include "quakedef.h"

extern qboolean mtexenabled; //johnfitz
extern cvar_t r_drawflat, gl_overbright_models, gl_fullbrights, r_lerpmodels, r_lerpmove; //johnfitz

//up to 16 color translated skins
gltexture_t *playertextures[MAX_SCOREBOARD]; //johnfitz -- changed to an array of pointers

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec3_t	shadevector;

#ifdef TRUE_LIGHTPOINT
extern lightpoint_args_t r_lightpoint_args;
#else
extern vec3_t	lightcolor; //johnfitz -- replaces "float shadelight" for lit support
#endif

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

#ifndef TRUE_LIGHTPOINT
extern	vec3_t			lightspot;
#endif

float	*shadedots = r_avertexnormal_dots[0];

float	entalpha; //johnfitz

qboolean	overbright; //johnfitz

qboolean shading = true; //johnfitz -- if false, disable vertex shading for various reasons (fullbright, r_lightmap, showtris, etc)

//johnfitz -- struct for passing lerp information to drawing functions
typedef struct {
	short pose1;
	short pose2;
	float blend;
	vec3_t origin;
	vec3_t angles;
} lerpdata_t;
//johnfitz

#ifdef SHADOW_VOLUMES
int r_num_shadowvolumes = 0;

typedef struct lerpedvert_s
{
	float position[3];
	float color[4];
	float texcoord[2];
} lerpedvert_t;

#define MAX_LERPED_VERTS	4000
#define MAX_LERPED_INDEXES	12000

lerpedvert_t r_lerped_verts[MAX_LERPED_VERTS];
unsigned int r_lerped_indexes[MAX_LERPED_INDEXES];

int r_num_lerped_verts;
int r_num_lerped_indexes;

void GL_LerpVerts (aliashdr_t *paliashdr, lerpdata_t lerpdata)
{
    trivertx_t *verts1, *verts2;
	int		*commands;
	float	blend, iblend;
	qboolean lerping;
	lerpedvert_t *lv;
	unsigned int *ndx;

	if (lerpdata.pose1 != lerpdata.pose2)
	{
		lerping = true;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts2  = verts1;
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
		verts2 += lerpdata.pose2 * paliashdr->poseverts;
		blend = lerpdata.blend;
		iblend = 1.0f - blend;
	}
	else // poses the same means either 1. the entity has paused its animation, or 2. r_lerpmodels is disabled
	{
		lerping = false;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
	}

	commands = (int *)((byte *)paliashdr + paliashdr->commands);

	lv = r_lerped_verts;
	ndx = r_lerped_indexes;

	r_num_lerped_verts = 0;
	r_num_lerped_indexes = 0;

	while (1)
	{
		// get the vertex count and primitive type
		int count = *commands++;
		int i;

		if (!count)
			break;		// done

		if (count < 0)
		{
			for (i = 2, count = -count; i < count; i++, ndx += 3, r_num_lerped_indexes += 3)
			{
				ndx[0] = r_num_lerped_verts + 0;
				ndx[1] = r_num_lerped_verts + (i - 1);
				ndx[2] = r_num_lerped_verts + i;
			}
		}
		else
		{
			for (i = 2; i < count; i++, ndx += 3, r_num_lerped_indexes += 3)
			{
				ndx[0] = r_num_lerped_verts + i - 2;
				ndx[1] = r_num_lerped_verts + ((i & 1) ? i : (i - 1));
				ndx[2] = r_num_lerped_verts + ((i & 1) ? (i - 1) : i);
			}
		}

		do
		{
			lv->texcoord[0] = ((float *)commands)[0];
			lv->texcoord[1] = ((float *)commands)[1];
			commands += 2;

			if (r_drawflat_cheatsafe)
			{
				srand(count * (unsigned int) commands);
				lv->color[0] = (rand () % 256) / 255.0f;
				lv->color[1] = (rand () % 256) / 255.0f;
				lv->color[2] = (rand () % 256) / 255.0f;
			}
#ifdef TRUE_LIGHTPOINT
			else if (lerping)
			{
				lv->color[0] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[0];
				lv->color[1] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[1];
				lv->color[2] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[2];
			}
			else
			{
				lv->color[0] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[0];
				lv->color[1] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[1];
				lv->color[2] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[2];
			}
#else
			else if (lerping)
			{
				lv->color[0] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[0];
				lv->color[1] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[1];
				lv->color[2] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[2];
			}
			else
			{
				lv->color[0] = shadedots[verts1->lightnormalindex] * lightcolor[0];
				lv->color[1] = shadedots[verts1->lightnormalindex] * lightcolor[1];
				lv->color[2] = shadedots[verts1->lightnormalindex] * lightcolor[2];
			}
#endif

			lv->color[3] = entalpha;

			if (lerping)
			{
				lv->position[0] = verts1->v[0] * iblend + verts2->v[0] * blend;
				lv->position[1] = verts1->v[1] * iblend + verts2->v[1] * blend;
				lv->position[2] = verts1->v[2] * iblend + verts2->v[2] * blend;
				verts1++;
				verts2++;
			}
			else
			{
				lv->position[0] = verts1->v[0];
				lv->position[1] = verts1->v[1];
				lv->position[2] = verts1->v[2];
				verts1++;
			}

			lv->position[0] = lv->position[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			lv->position[1] = lv->position[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			lv->position[2] = lv->position[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			r_num_lerped_verts++;
			lv++;
		} while (--count);
	}
}

void GL_DrawAliasFrame (aliashdr_t *paliashdr, lerpdata_t lerpdata)
{
	int		*commands;
	int		count;
	lerpedvert_t *lv;

	commands = (int *)((byte *)paliashdr + paliashdr->commands);
	lv = r_lerped_verts;

	while (1)
	{
		// get the vertex count and primitive type
		count = *commands++;

		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else glBegin (GL_TRIANGLE_STRIP);

		do
		{
			if (mtexenabled)
			{
				GL_MTexCoord2fFunc (TEXTURE0, lv->texcoord[0], lv->texcoord[1]);
				GL_MTexCoord2fFunc (TEXTURE1, lv->texcoord[0], lv->texcoord[1]);
			}
			else glTexCoord2fv (lv->texcoord);

			if (shading)
				glColor4fv (lv->color);

			glVertex3fv (lv->position);

			commands += 2;
			lv++;
		} while (--count);

		glEnd ();
	}

	rs_aliaspasses += paliashdr->numtris;
}


typedef struct edgeDef_s
{
	int		i2;
	int		facing;
} edgeDef_t;

#define	MAX_EDGE_DEFS	32

static	edgeDef_t	edgeDefs[MAX_LERPED_VERTS][MAX_EDGE_DEFS];
static	int			numEdgeDefs[MAX_LERPED_VERTS];
static	int			facing[MAX_LERPED_INDEXES / 3];

void R_AddEdgeDef (int i1, int i2, int facing)
{
	int	c = numEdgeDefs[i1];

	if (c == MAX_EDGE_DEFS)
	{
		// overflow
		return;
	}

	edgeDefs[i1][c].i2 = i2;
	edgeDefs[i1][c].facing = facing;

	numEdgeDefs[i1]++;
}

void R_RenderShadowEdges (void)
{
	int		i;
	int		c, c2;
	int		j, k;
	int		i2;
	int		hit[2];

	// an edge is NOT a silhouette edge if its face doesn't face the light,
	// or if it has a reverse paired edge that also faces the light.
	// A well behaved polyhedron would have exactly two faces for each edge,
	// but lots of models have dangling edges or overfanned edges
	glBegin (GL_QUADS);

	for (i = 0; i < r_num_lerped_verts; i++)
	{
		c = numEdgeDefs[i];

		for (j = 0; j < c; j++)
		{
			if (!edgeDefs[i][j].facing)
				continue;

			hit[0] = 0;
			hit[1] = 0;

			i2 = edgeDefs[i][j].i2;
			c2 = numEdgeDefs[i2];

			for (k = 0; k < c2; k++)
			{
				if (edgeDefs[i2][k].i2 == i)
				{
					hit[edgeDefs[i2][k].facing]++;
				}
			}

			// if it doesn't share the edge with another front facing
			// triangle, it is a sil edge
			if (hit[1] == 0)
			{
				glVertex3fv (r_lerped_verts[i + r_num_lerped_verts].position);
				glVertex3fv (r_lerped_verts[i2 + r_num_lerped_verts].position);
				glVertex3fv (r_lerped_verts[i2].position);
				glVertex3fv (r_lerped_verts[i].position);
			}
		}
	}

	glEnd ();
}


void RB_ShadowTessEnd (lerpdata_t *lerpdata)
{
	int		i;
	int		numTris;
	vec3_t	lightDir = {400, 0, 400}; // this mimics the approximate angle of classic GLQuake shadows

	if (lerpdata->angles[0] || lerpdata->angles[1] || lerpdata->angles[2])
	{
		vec3_t forward, right, up, temp;

		AngleVectors (lerpdata->angles, forward, right, up);
		VectorCopy (lightDir, temp);

		lightDir[0] = DotProduct (temp, forward);
		lightDir[1] = -DotProduct (temp, right);
		lightDir[2] = DotProduct (temp, up);
	}

	VectorNormalize (lightDir);

	// project vertexes away from light direction
	for (i = 0; i < r_num_lerped_verts; i++)
		VectorMA (r_lerped_verts[i].position, -512, lightDir, r_lerped_verts[r_num_lerped_verts + i].position);

	// decide which triangles face the light
	memset (numEdgeDefs, 0, 4 * r_num_lerped_verts);

	numTris = r_num_lerped_indexes / 3;

	for (i = 0; i < numTris; i++ )
	{
		int		i1, i2, i3;
		vec3_t	d1, d2, normal;
		float	*v1, *v2, *v3;
		float	d;

		i1 = r_lerped_indexes[i * 3 + 0];
		i2 = r_lerped_indexes[i * 3 + 1];
		i3 = r_lerped_indexes[i * 3 + 2];

		v1 = r_lerped_verts[i1].position;
		v2 = r_lerped_verts[i2].position;
		v3 = r_lerped_verts[i3].position;

		VectorSubtract (v2, v1, d1);
		VectorSubtract (v3, v1, d2);
		CrossProduct (d1, d2, normal);

		d = DotProduct (normal, lightDir);

		if (d > 0)
			facing[i] = 1;
		else facing[i] = 0;

		// create the edges
		R_AddEdgeDef (i1, i2, facing[i]);
		R_AddEdgeDef (i2, i3, facing[i]);
		R_AddEdgeDef (i3, i1, facing[i]);
	}

	// draw the silhouette edges
	glEnable (GL_CULL_FACE);
	glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask (GL_FALSE);

	glEnable (GL_STENCIL_TEST);
	glStencilFunc (GL_ALWAYS, 1, 255);

	// because fitzquake flipped the front-face and cull, we need to do the same here
	// we could use separate stencil here for a slightly more sensible one-pass draw which would be easier on the CPU
	glCullFace (GL_FRONT);
	glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);

	R_RenderShadowEdges ();

	glCullFace (GL_BACK);
	glStencilOp (GL_KEEP, GL_KEEP, GL_DECR);

	R_RenderShadowEdges ();

	// reenable writing to the color buffer
	glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask (GL_TRUE);
	glDisable (GL_STENCIL_TEST);
	glCullFace (GL_BACK);
}

/*
=================
RB_ShadowFinish

Darken everything that is in a shadow volume.
We have to delay this until everything has been shadowed,
because otherwise shadows from different body parts would
overlap and double darken.
=================
*/
void RB_ShadowFinish (void)
{
	// if no shadows were drawn there's no work to be done here
	if (!r_num_shadowvolumes) return;

	glEnable (GL_STENCIL_TEST);
	glStencilFunc (GL_NOTEQUAL, 0, 255);

	glDisable (GL_CULL_FACE);
	glDisable (GL_TEXTURE_2D);

	glLoadIdentity ();

	glColor3f (0.6f, 0.6f, 0.6f);
	glEnable (GL_BLEND);
	glBlendFunc (GL_DST_COLOR, GL_ZERO);
	glDepthMask (1);

	glBegin (GL_QUADS);
	glVertex3f (-100, 100, -10);
	glVertex3f (100, 100, -10);
	glVertex3f (100, -100, -10);
	glVertex3f (-100, -100, -10);
	glEnd ();

	glColor4f (1, 1, 1, 1);
	glDisable (GL_STENCIL_TEST);
	glEnable (GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_BLEND);

	// for the next frame
	r_num_shadowvolumes = 0;
}
#else
/*
=============
GL_DrawAliasFrame -- johnfitz -- rewritten to support colored light, lerping, entalpha, multitexture, and r_drawflat
=============
*/
void GL_DrawAliasFrame (aliashdr_t *paliashdr, lerpdata_t lerpdata)
{
	float 	vertcolor[4];
    trivertx_t *verts1, *verts2;
	int		*commands;
	int		count;
	float	u,v;
	float	blend, iblend;
	qboolean lerping;

	if (lerpdata.pose1 != lerpdata.pose2)
	{
		lerping = true;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts2  = verts1;
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
		verts2 += lerpdata.pose2 * paliashdr->poseverts;
		blend = lerpdata.blend;
		iblend = 1.0f - blend;
	}
	else // poses the same means either 1. the entity has paused its animation, or 2. r_lerpmodels is disabled
	{
		lerping = false;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
	}

	commands = (int *)((byte *)paliashdr + paliashdr->commands);

	vertcolor[3] = entalpha; //never changes, so there's no need to put this inside the loop

	while (1)
	{
		// get the vertex count and primitive type
		count = *commands++;
		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			u = ((float *)commands)[0];
			v = ((float *)commands)[1];
			if (mtexenabled)
			{
				GL_MTexCoord2fFunc (TEXTURE0, u, v);
				GL_MTexCoord2fFunc (TEXTURE1, u, v);
			}
			else
				glTexCoord2f (u, v);

			commands += 2;

			if (shading)
			{
				if (r_drawflat_cheatsafe)
				{
					srand(count * (unsigned int) commands);
					glColor3f (rand()%256/255.0, rand()%256/255.0, rand()%256/255.0);
				}
#ifdef TRUE_LIGHTPOINT
				else if (lerping)
				{
					vertcolor[0] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[0];
					vertcolor[1] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[1];
					vertcolor[2] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * r_lightpoint_args.color[2];
					glColor4fv (vertcolor);
				}
				else
				{
					vertcolor[0] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[0];
					vertcolor[1] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[1];
					vertcolor[2] = shadedots[verts1->lightnormalindex] * r_lightpoint_args.color[2];
					glColor4fv (vertcolor);
				}
#else
				else if (lerping)
				{
					vertcolor[0] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[0];
					vertcolor[1] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[1];
					vertcolor[2] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[2];
					glColor4fv (vertcolor);
				}
				else
				{
					vertcolor[0] = shadedots[verts1->lightnormalindex] * lightcolor[0];
					vertcolor[1] = shadedots[verts1->lightnormalindex] * lightcolor[1];
					vertcolor[2] = shadedots[verts1->lightnormalindex] * lightcolor[2];
					glColor4fv (vertcolor);
				}
#endif
			}

			if (lerping)
			{
				glVertex3f (verts1->v[0]*iblend + verts2->v[0]*blend,
							verts1->v[1]*iblend + verts2->v[1]*blend,
							verts1->v[2]*iblend + verts2->v[2]*blend);
				verts1++;
				verts2++;
			}
			else
			{
				glVertex3f (verts1->v[0], verts1->v[1], verts1->v[2]);
				verts1++;
			}
		} while (--count);

		glEnd ();
	}

	rs_aliaspasses += paliashdr->numtris;
}
#endif


/*
=================
R_SetupAliasFrame -- johnfitz -- rewritten to support lerping
=================
*/
void R_SetupAliasFrame (aliashdr_t *paliashdr, int frame, lerpdata_t *lerpdata)
{
	entity_t		*e = currententity;
	int				posenum, numposes;

	if ((frame >= paliashdr->numframes) || (frame < 0))
	{
		Con_DPrintf ("R_AliasSetupFrame: no such frame %d\n", frame);
		frame = 0;
	}

	posenum = paliashdr->frames[frame].firstpose;
	numposes = paliashdr->frames[frame].numposes;

	if (numposes > 1)
	{
		e->lerptime = paliashdr->frames[frame].interval;
		posenum += (int)(cl.time / e->lerptime) % numposes;
	}
	else
		e->lerptime = 0.1;

	if (e->lerpflags & LERP_RESETANIM) //kill any lerp in progress
	{
		e->lerpstart = 0;
		e->previouspose = posenum;
		e->currentpose = posenum;
		e->lerpflags -= LERP_RESETANIM;
	}
	else if (e->currentpose != posenum) // pose changed, start new lerp
	{
		if (e->lerpflags & LERP_RESETANIM2) //defer lerping one more time
		{
			e->lerpstart = 0;
			e->previouspose = posenum;
			e->currentpose = posenum;
			e->lerpflags -= LERP_RESETANIM2;
		}
		else
		{
			e->lerpstart = cl.time;
			e->previouspose = e->currentpose;
			e->currentpose = posenum;
		}
	}

	//set up values
	if (r_lerpmodels.value && !(e->model->flags & MOD_NOLERP && r_lerpmodels.value != 2))
	{
		if (e->lerpflags & LERP_FINISH && numposes == 1)
			lerpdata->blend = CLAMP (0, (cl.time - e->lerpstart) / (e->lerpfinish - e->lerpstart), 1);
		else
			lerpdata->blend = CLAMP (0, (cl.time - e->lerpstart) / e->lerptime, 1);
		lerpdata->pose1 = e->previouspose;
		lerpdata->pose2 = e->currentpose;
	}
	else //don't lerp
	{
		lerpdata->blend = 1;
		lerpdata->pose1 = posenum;
		lerpdata->pose2 = posenum;
	}
}

/*
=================
R_SetupEntityTransform -- johnfitz -- set up transform part of lerpdata
=================
*/
void R_SetupEntityTransform (entity_t *e, lerpdata_t *lerpdata)
{
	float blend;
	vec3_t d;
	int i;

	// if LERP_RESETMOVE, kill any lerps in progress
	if (e->lerpflags & LERP_RESETMOVE)
	{
		e->movelerpstart = 0;
		VectorCopy (e->origin, e->previousorigin);
		VectorCopy (e->origin, e->currentorigin);
		VectorCopy (e->angles, e->previousangles);
		VectorCopy (e->angles, e->currentangles);
		e->lerpflags -= LERP_RESETMOVE;
	}
	else if (!VectorCompare (e->origin, e->currentorigin) || !VectorCompare (e->angles, e->currentangles)) // origin/angles changed, start new lerp
	{
		e->movelerpstart = cl.time;
		VectorCopy (e->currentorigin, e->previousorigin);
		VectorCopy (e->origin,  e->currentorigin);
		VectorCopy (e->currentangles, e->previousangles);
		VectorCopy (e->angles,  e->currentangles);
	}

	//set up values
	if (r_lerpmove.value && e != &cl.viewent && e->lerpflags & LERP_MOVESTEP)
	{
		if (e->lerpflags & LERP_FINISH)
			blend = CLAMP (0, (cl.time - e->movelerpstart) / (e->lerpfinish - e->movelerpstart), 1);
		else
			blend = CLAMP (0, (cl.time - e->movelerpstart) / 0.1, 1);

		//translation
		VectorSubtract (e->currentorigin, e->previousorigin, d);
		lerpdata->origin[0] = e->previousorigin[0] + d[0] * blend;
		lerpdata->origin[1] = e->previousorigin[1] + d[1] * blend;
		lerpdata->origin[2] = e->previousorigin[2] + d[2] * blend;

		//rotation
		VectorSubtract (e->currentangles, e->previousangles, d);
		for (i = 0; i < 3; i++)
		{
			if (d[i] > 180)  d[i] -= 360;
			if (d[i] < -180) d[i] += 360;
		}
		lerpdata->angles[0] = e->previousangles[0] + d[0] * blend;
		lerpdata->angles[1] = e->previousangles[1] + d[1] * blend;
		lerpdata->angles[2] = e->previousangles[2] + d[2] * blend;
	}
	else //don't lerp
	{
		VectorCopy (e->origin, lerpdata->origin);
		VectorCopy (e->angles, lerpdata->angles);
	}
}

/*
=================
R_SetupAliasLighting -- johnfitz -- broken out from R_DrawAliasModel and rewritten
=================
*/
void R_SetupAliasLighting (entity_t	*e)
{
	vec3_t		dist;
	float		add;
	int			i;

	R_LightPoint (e->origin);

	//add dlights
	for (i=0 ; i<MAX_DLIGHTS ; i++)
	{
		if (cl_dlights[i].die >= cl.time)
		{
			VectorSubtract (currententity->origin, cl_dlights[i].origin, dist);
			add = cl_dlights[i].radius - Length(dist);
#ifdef EXTRA_DLIGHTS
			if (add > 0)
			{
#ifdef TRUE_LIGHTPOINT
				r_lightpoint_args.color[0] += (add * cl_dlights[i].rgb[0]) * (1.0f / 255.0f);
				r_lightpoint_args.color[1] += (add * cl_dlights[i].rgb[1]) * (1.0f / 255.0f);
				r_lightpoint_args.color[2] += (add * cl_dlights[i].rgb[2]) * (1.0f / 255.0f);
#else
				lightcolor[0] += (add * cl_dlights[i].rgb[0]) * (1.0f / 255.0f);
				lightcolor[1] += (add * cl_dlights[i].rgb[1]) * (1.0f / 255.0f);
				lightcolor[2] += (add * cl_dlights[i].rgb[2]) * (1.0f / 255.0f);
#endif
			}
#else
			if (add > 0)
#ifdef TRUE_LIGHTPOINT
				VectorMA (r_lightpoint_args.color, add, cl_dlights[i].color, lightcolor);
#else
				VectorMA (lightcolor, add, cl_dlights[i].color, lightcolor);
#endif
#endif
		}
	}

	// minimum light value on gun (24)
	if (e == &cl.viewent)
	{
#ifdef TRUE_LIGHTPOINT
		add = 72.0f - (r_lightpoint_args.color[0] + r_lightpoint_args.color[1] + r_lightpoint_args.color[2]);
		if (add > 0.0f)
		{
			r_lightpoint_args.color[0] += add / 3.0f;
			r_lightpoint_args.color[1] += add / 3.0f;
			r_lightpoint_args.color[2] += add / 3.0f;
		}
#else
		add = 72.0f - (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add > 0.0f)
		{
			lightcolor[0] += add / 3.0f;
			lightcolor[1] += add / 3.0f;
			lightcolor[2] += add / 3.0f;
		}
#endif
	}

	// minimum light value on players (8)
	if (currententity > cl_entities && currententity <= cl_entities + cl.maxclients)
	{
#ifdef TRUE_LIGHTPOINT
		add = 24.0f - (r_lightpoint_args.color[0] + r_lightpoint_args.color[1] + r_lightpoint_args.color[2]);
		if (add > 0.0f)
		{
			r_lightpoint_args.color[0] += add / 3.0f;
			r_lightpoint_args.color[1] += add / 3.0f;
			r_lightpoint_args.color[2] += add / 3.0f;
		}
#else
		add = 24.0f - (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add > 0.0f)
		{
			lightcolor[0] += add / 3.0f;
			lightcolor[1] += add / 3.0f;
			lightcolor[2] += add / 3.0f;
		}
#endif
	}

	// clamp lighting so it doesn't overbright as much (96)
	if (overbright)
	{
#ifdef TRUE_LIGHTPOINT
		add = 288.0f / (r_lightpoint_args.color[0] + r_lightpoint_args.color[1] + r_lightpoint_args.color[2]);
		if (add < 1.0f)
			VectorScale(r_lightpoint_args.color, add, r_lightpoint_args.color);
#else
		add = 288.0f / (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add < 1.0f)
			VectorScale(lightcolor, add, lightcolor);
#endif
	}

	//hack up the brightness when fullbrights but no overbrights (256)
	if (gl_fullbrights.value && !gl_overbright_models.value)
		if (e->model->flags & MOD_FBRIGHTHACK)
		{
#ifdef TRUE_LIGHTPOINT
			r_lightpoint_args.color[0] = 256.0f;
			r_lightpoint_args.color[1] = 256.0f;
			r_lightpoint_args.color[2] = 256.0f;
#else
			lightcolor[0] = 256.0f;
			lightcolor[1] = 256.0f;
			lightcolor[2] = 256.0f;
#endif
		}

	shadedots = r_avertexnormal_dots[((int)(e->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
#ifdef TRUE_LIGHTPOINT
	VectorScale(r_lightpoint_args.color, 1.0f / 200.0f, r_lightpoint_args.color);
#else
	VectorScale(lightcolor, 1.0f / 200.0f, lightcolor);
#endif
}

/*
=================
R_DrawAliasModel -- johnfitz -- almost completely rewritten
=================
*/
void R_DrawAliasModel (entity_t *e)
{
	aliashdr_t	*paliashdr;
	vec3_t		mins, maxs;
	int			i, anim;
	gltexture_t	*tx, *fb;
	lerpdata_t	lerpdata;

	//
	// setup pose/lerp data -- do it first so we don't miss updates due to culling
	//
	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);

	//
	// cull it
	//
	if (R_CullModelForEntity(e))
		return;

	//
	// transform it
	//
    glPushMatrix ();
	R_RotateForEntity (lerpdata.origin, lerpdata.angles);
#ifndef SHADOW_VOLUMES
	glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	glScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);
#endif

	//
	// random stuff
	//
	if (gl_smoothmodels.value && !r_drawflat_cheatsafe)
		glShadeModel (GL_SMOOTH);
	if (gl_affinemodels.value)
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	overbright = gl_overbright_models.value;
	shading = true;

	//
	// set up for alpha blending
	//
	if (r_drawflat_cheatsafe || r_lightmap_cheatsafe) //no alpha in drawflat or lightmap mode
		entalpha = 1;
	else
		entalpha = ENTALPHA_DECODE(e->alpha);
	if (entalpha == 0)
		goto cleanup;
	if (entalpha < 1)
	{
		if (!gl_texture_env_combine) overbright = false; //overbright can't be done in a single pass without combiners
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
	}

	//
	// set up lighting
	//
	rs_aliaspolys += paliashdr->numtris;
	R_SetupAliasLighting (e);

	//
	// set up textures
	//
	GL_DisableMultitexture();
	anim = (int)(cl.time*10) & 3;
	tx = paliashdr->gltextures[e->skinnum][anim];
	fb = paliashdr->fbtextures[e->skinnum][anim];
	if (e->colormap != vid.colormap && !gl_nocolors.value)
	{
		i = e - cl_entities;
		if (i >= 1 && i<=cl.maxclients /* && !strcmp (currententity->model->name, "progs/player.mdl") */)
		    tx = playertextures[i - 1];
	}
	if (!gl_fullbrights.value)
		fb = NULL;

#ifdef SHADOW_VOLUMES
	// pre-lerp all verts so that we can reuse the data over multiple passes
	GL_LerpVerts (paliashdr, lerpdata);
#endif

	//
	// draw it
	//
	if (r_drawflat_cheatsafe)
	{
		glDisable (GL_TEXTURE_2D);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		glEnable (GL_TEXTURE_2D);
		srand((int) (cl.time * 1000)); //restore randomness
	}
	else if (r_fullbright_cheatsafe)
	{
		GL_Bind (tx);
		shading = false;
		glColor4f(1,1,1,entalpha);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		if (fb)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_Bind(fb);
			glEnable(GL_BLEND);
			glBlendFunc (GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
			glColor3f(entalpha,entalpha,entalpha);
			Fog_StartAdditive ();
			GL_DrawAliasFrame (paliashdr, lerpdata);
			Fog_StopAdditive ();
			glDepthMask(GL_TRUE);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_BLEND);
		}
	}
	else if (r_lightmap_cheatsafe)
	{
		glDisable (GL_TEXTURE_2D);
		shading = false;
		glColor3f(1,1,1);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		glEnable (GL_TEXTURE_2D);
	}
	else if (overbright)
	{
		if  (gl_texture_env_combine && gl_mtexable && gl_texture_env_add && fb) //case 1: everything in one pass
		{
			GL_Bind (tx);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);
			GL_EnableMultitexture(); // selects TEXTURE1
			GL_Bind (fb);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			glEnable(GL_BLEND);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			glDisable(GL_BLEND);
			GL_DisableMultitexture();
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		else if (gl_texture_env_combine) //case 2: overbright in one pass, then fullbright pass
		{
		// first pass
			GL_Bind(tx);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		// second pass
			if (fb)
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind(fb);
				glEnable(GL_BLEND);
				glBlendFunc (GL_ONE, GL_ONE);
				glDepthMask(GL_FALSE);
				shading = false;
				glColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				glDepthMask(GL_TRUE);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_BLEND);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
		}
		else //case 3: overbright in two passes, then fullbright pass
		{
		// first pass
			GL_Bind(tx);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_DrawAliasFrame (paliashdr, lerpdata);
		// second pass -- additive with black fog, to double the object colors but not the fog color
			glEnable(GL_BLEND);
			glBlendFunc (GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
			Fog_StartAdditive ();
			GL_DrawAliasFrame (paliashdr, lerpdata);
			Fog_StopAdditive ();
			glDepthMask(GL_TRUE);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_BLEND);
		// third pass
			if (fb)
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind(fb);
				glEnable(GL_BLEND);
				glBlendFunc (GL_ONE, GL_ONE);
				glDepthMask(GL_FALSE);
				shading = false;
				glColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				glDepthMask(GL_TRUE);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_BLEND);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
		}
	}
	else
	{
		if (gl_mtexable && gl_texture_env_add && fb) //case 4: fullbright mask using multitexture
		{
			GL_DisableMultitexture(); // selects TEXTURE0
			GL_Bind (tx);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_EnableMultitexture(); // selects TEXTURE1
			GL_Bind (fb);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			glEnable(GL_BLEND);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			glDisable(GL_BLEND);
			GL_DisableMultitexture();
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		else //case 5: fullbright mask without multitexture
		{
		// first pass
			GL_Bind(tx);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_DrawAliasFrame (paliashdr, lerpdata);
		// second pass
			if (fb)
			{
				GL_Bind(fb);
				glEnable(GL_BLEND);
				glBlendFunc (GL_ONE, GL_ONE);
				glDepthMask(GL_FALSE);
				shading = false;
				glColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				glDepthMask(GL_TRUE);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_BLEND);
			}
		}
	}

cleanup:
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel (GL_FLAT);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glColor3f(1,1,1);

#ifdef SHADOW_VOLUMES
	if (!(e == &cl.viewent || (e->model->flags & MOD_NOSHADOW)) && r_shadows.value)
	{
		RB_ShadowTessEnd (&lerpdata);
		r_num_shadowvolumes++;
	}
#endif

	glPopMatrix ();
}

#ifndef SHADOW_VOLUMES
//johnfitz -- values for shadow matrix
#define SHADOW_SKEW_X -0.7 //skew along x axis. -0.7 to mimic glquake shadows
#define SHADOW_SKEW_Y 0 //skew along y axis. 0 to mimic glquake shadows
#define SHADOW_VSCALE 0 //0=completely flat
#define SHADOW_HEIGHT 0.1 //how far above the floor to render the shadow
//johnfitz

/*
=============
GL_DrawAliasShadow -- johnfitz -- rewritten

TODO: orient shadow onto "lightplane" (a global mplane_t*)
=============
*/
void GL_DrawAliasShadow (entity_t *e)
{
	float	shadowmatrix[16] = {1,				0,				0,				0,
								0,				1,				0,				0,
								SHADOW_SKEW_X,	SHADOW_SKEW_Y,	SHADOW_VSCALE,	0,
								0,				0,				SHADOW_HEIGHT,	1};

	float		lheight;
	aliashdr_t	*paliashdr;
	vec3_t		mins, maxs;
	lerpdata_t	lerpdata;

	if (R_CullModelForEntity(e))
		return;

	if (e == &cl.viewent || e->model->flags & MOD_NOSHADOW)
		return;

	entalpha = ENTALPHA_DECODE(e->alpha);
	if (entalpha == 0) return;

	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);
	R_LightPoint (e->origin);

#ifdef TRUE_LIGHTPOINT
	lheight = currententity->origin[2] - r_lightpoint_args.spot[2];
#else
	lheight = currententity->origin[2] - lightspot[2];
#endif

#ifdef STENCIL_SHADOWS
	glEnable (GL_STENCIL_TEST);
	glStencilFunc (GL_EQUAL, 1, 2);
	glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
#endif

// set up matrix
    glPushMatrix ();
	glTranslatef (lerpdata.origin[0],  lerpdata.origin[1],  lerpdata.origin[2]);
	glTranslatef (0,0,-lheight);
	glMultMatrixf (shadowmatrix);
	glTranslatef (0,0,lheight);
	glRotatef (lerpdata.angles[1],  0, 0, 1);
	glRotatef (-lerpdata.angles[0],  0, 1, 0);
	glRotatef (lerpdata.angles[2],  1, 0, 0);
	glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	glScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);

// draw it
	glDepthMask(GL_FALSE);
	glEnable (GL_BLEND);
	GL_DisableMultitexture ();
	glDisable (GL_TEXTURE_2D);
	shading = false;
	glColor4f(0,0,0,entalpha * 0.5);
	GL_DrawAliasFrame (paliashdr, lerpdata);
	glEnable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	glDepthMask(GL_TRUE);

#ifdef STENCIL_SHADOWS
	glDisable (GL_STENCIL_TEST);
#endif

//clean up
	glPopMatrix ();
}
#endif

/*
=================
R_DrawAliasModel_ShowTris -- johnfitz
=================
*/
void R_DrawAliasModel_ShowTris (entity_t *e)
{
	aliashdr_t	*paliashdr;
	vec3_t		mins, maxs;
	lerpdata_t	lerpdata;

	if (R_CullModelForEntity(e))
		return;

	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);

    glPushMatrix ();
	R_RotateForEntity (lerpdata.origin,lerpdata.angles);
	glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	glScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);

	shading = false;
	glColor3f(1,1,1);
	GL_DrawAliasFrame (paliashdr, lerpdata);

	glPopMatrix ();
}

