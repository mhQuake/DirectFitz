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
// r_light.c

#include "quakedef.h"

int	r_dlightframecount;

extern cvar_t r_flatlightstyles; //johnfitz

/*
==================
R_AnimateLight
==================
*/
void R_AnimateLight (void)
{
	int			i,j,k;

//
// light animations
// 'm' is normal light, 'a' is no light, 'z' is double bright
	i = (int)(cl.time*10);
	for (j=0 ; j<MAX_LIGHTSTYLES ; j++)
	{
		if (!cl_lightstyle[j].length)
		{
			d_lightstylevalue[j] = 256;
			continue;
		}
		//johnfitz -- r_flatlightstyles
		if (r_flatlightstyles.value == 2)
			k = cl_lightstyle[j].peak - 'a';
		else if (r_flatlightstyles.value == 1)
			k = cl_lightstyle[j].average - 'a';
		else
		{
			k = i % cl_lightstyle[j].length;
			k = cl_lightstyle[j].map[k] - 'a';
		}
		d_lightstylevalue[j] = k*22;
		//johnfitz
	}
}

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING (gl_flashblend 1)

=============================================================================
*/

void AddLightBlend (float r, float g, float b, float a2)
{
	float	a;

	v_blend[3] = a = v_blend[3] + a2*(1-v_blend[3]);

	a2 = a2/a;

	v_blend[0] = v_blend[1]*(1-a2) + r*a2;
	v_blend[1] = v_blend[1]*(1-a2) + g*a2;
	v_blend[2] = v_blend[2]*(1-a2) + b*a2;
}

void R_RenderDlight (dlight_t *light)
{
	int		i, j;
	float	a;
	vec3_t	v;
	float	rad;

	rad = light->radius * 0.35;

	VectorSubtract (light->origin, r_origin, v);
	if (Length (v) < rad)
	{	// view is inside the dlight
		AddLightBlend (1, 0.5, 0, light->radius * 0.0003);
		return;
	}

	glBegin (GL_TRIANGLE_FAN);
	glColor3f (0.2,0.1,0.0);
	for (i=0 ; i<3 ; i++)
		v[i] = light->origin[i] - vpn[i]*rad;
	glVertex3fv (v);
	glColor3f (0,0,0);
	for (i=16 ; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;
		for (j=0 ; j<3 ; j++)
			v[j] = light->origin[j] + vright[j]*cos(a)*rad
				+ vup[j]*sin(a)*rad;
		glVertex3fv (v);
	}
	glEnd ();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!gl_flashblend.value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	glDepthMask (0);
	glDisable (GL_TEXTURE_2D);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE);

	l = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;
		R_RenderDlight (l);
	}

	glColor3f (1,1,1);
	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (1);
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights -- johnfitz -- rewritten to use LordHavoc's lighting speedup
=============
*/
#ifdef EXTRA_DLIGHTS
void R_MarkLights (dlight_t *light, int num, mnode_t *node)
#else
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
#endif
{
	mplane_t	*splitplane;
	msurface_t	*surf;
	vec3_t		impact;
	float		dist, l, maxdist;
	int			i, j, s, t;

start:

	if (node->contents < 0)
		return;

	splitplane = node->plane;
#ifdef CORRECT_DLIGHT_ORIGINS
	if (splitplane->type < 3)
		dist = light->transformed[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (light->transformed, splitplane->normal) - splitplane->dist;
#else
	if (splitplane->type < 3)
		dist = light->origin[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;
#endif

	if (dist > light->radius)
	{
		node = node->children[0];
		goto start;
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto start;
	}

	maxdist = light->radius*light->radius;
// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
#ifdef CORRECT_DLIGHT_ORIGINS
		for (j=0 ; j<3 ; j++)
			impact[j] = light->transformed[j] - surf->plane->normal[j]*dist;
#else
		for (j=0 ; j<3 ; j++)
			impact[j] = light->origin[j] - surf->plane->normal[j]*dist;
#endif
		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;if (s < 0) s = 0;else if (s > surf->extents[0]) s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;if (t < 0) t = 0;else if (t > surf->extents[1]) t = surf->extents[1];
		t = l - t;
		// compare to minimum light
		if ((s*s+t*t+dist*dist) < maxdist)
		{
#ifdef EXTRA_DLIGHTS
			if (surf->dlightframe != r_dlightframecount) // not dynamic until now
			{
				memset (surf->dlightbits, 0, sizeof (surf->dlightbits));
				surf->dlightframe = r_dlightframecount;
			}

			surf->dlightbits[num >> 5] |= 1 << (num & 31);
#else
			if (surf->dlightframe != r_dlightframecount) // not dynamic until now
			{
				surf->dlightbits = bit;
				surf->dlightframe = r_dlightframecount;
			}
			else // already dynamic
				surf->dlightbits |= bit;
#endif
		}
	}

#ifdef EXTRA_DLIGHTS
	if (node->children[0]->contents >= 0)
		R_MarkLights (light, num, node->children[0]);
	if (node->children[1]->contents >= 0)
		R_MarkLights (light, num, node->children[1]);
#else
	if (node->children[0]->contents >= 0)
		R_MarkLights (light, bit, node->children[0]);
	if (node->children[1]->contents >= 0)
		R_MarkLights (light, bit, node->children[1]);
#endif
}

/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (gl_flashblend.value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = cl_dlights;

	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;

#ifdef CORRECT_DLIGHT_ORIGINS
	    VectorCopy (l->origin, l->transformed);
#endif

#ifdef EXTRA_DLIGHTS
		R_MarkLights ( l, i, cl.worldmodel->nodes );
#else
		R_MarkLights ( l, 1<<i, cl.worldmodel->nodes );
#endif
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

#ifdef TRUE_LIGHTPOINT
lightpoint_args_t r_lightpoint_args;
#else
mplane_t		*lightplane;
vec3_t			lightspot;
vec3_t			lightcolor; //johnfitz -- lit support via lordhavoc
#endif

/*
=============
RecursiveLightPoint -- johnfitz -- replaced entire function for lit support via lordhavoc
=============
*/
#ifdef TRUE_LIGHTPOINT
int RecursiveLightPoint (lightpoint_args_t *args, mnode_t *node, vec3_t start, vec3_t end)
#else
int RecursiveLightPoint (vec3_t color, mnode_t *node, vec3_t start, vec3_t end)
#endif
{
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything

// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front = DotProduct(start, node->plane->normal) - node->plane->dist;
		back = DotProduct(end, node->plane->normal) - node->plane->dist;
	}

	// LordHavoc: optimized recursion
	if ((back < 0) == (front < 0))
//		return RecursiveLightPoint (color, node->children[front < 0], start, end);
	{
		node = node->children[front < 0];
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side
#ifdef TRUE_LIGHTPOINT
	if (RecursiveLightPoint (args, node->children[front < 0], start, mid))
#else
	if (RecursiveLightPoint (color, node->children[front < 0], start, mid))
#endif
		return true;	// hit something
	else
	{
		int i, ds, dt;
		msurface_t *surf;
	// check for impact on this node
#ifdef TRUE_LIGHTPOINT
		VectorCopy (mid, args->spot);
		args->plane = node->plane;
#else
		VectorCopy (mid, lightspot);
		lightplane = node->plane;
#endif

		surf = cl.worldmodel->surfaces + node->firstsurface;

		for (i = 0;i < node->numsurfaces;i++, surf++)
		{
			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			ds = (int) ((float) DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int) ((float) DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;

			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];

			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->samples)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte *lightmap;
				int maps, line3, dsfrac = ds & 15, dtfrac = dt & 15, r00 = 0, g00 = 0, b00 = 0, r01 = 0, g01 = 0, b01 = 0, r10 = 0, g10 = 0, b10 = 0, r11 = 0, g11 = 0, b11 = 0;
				float scale;
				line3 = ((surf->extents[0]>>4)+1)*3;

				lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4))*3; // LordHavoc: *3 for color

				for (maps = 0;maps < MAXLIGHTMAPS && surf->styles[maps] != 255;maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float) lightmap[      0] * scale;g00 += (float) lightmap[      1] * scale;b00 += (float) lightmap[2] * scale;
					r01 += (float) lightmap[      3] * scale;g01 += (float) lightmap[      4] * scale;b01 += (float) lightmap[5] * scale;
					r10 += (float) lightmap[line3+0] * scale;g10 += (float) lightmap[line3+1] * scale;b10 += (float) lightmap[line3+2] * scale;
					r11 += (float) lightmap[line3+3] * scale;g11 += (float) lightmap[line3+4] * scale;b11 += (float) lightmap[line3+5] * scale;
					lightmap += ((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3; // LordHavoc: *3 for colored lighting
				}

#ifdef TRUE_LIGHTPOINT
				args->color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				args->color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				args->color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
#else
				color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
#endif
			}

#ifdef TRUE_LIGHTPOINT
			args->surf = surf;
#endif
			return true; // success
		}

	// go down back side
#ifdef TRUE_LIGHTPOINT
		return RecursiveLightPoint (args, node->children[front >= 0], mid, end);
#else
		return RecursiveLightPoint (color, node->children[front >= 0], mid, end);
#endif
	}
}


#ifdef TRUE_LIGHTPOINT
void R_InverseTransform (float *m, float *out, const float *in)
{
	// copy off "in" so that we can use the same pointer for out and in
	float in2[3] = {in[0], in[1], in[2]};

	// http://content.gpwiki.org/index.php/MathGem:Fast_Matrix_Inversion
	out[0] = DotProduct (in2, (&m[0])) - DotProduct ((&m[0]), (&m[12]));
	out[1] = DotProduct (in2, (&m[4])) - DotProduct ((&m[4]), (&m[12]));
	out[2] = DotProduct (in2, (&m[8])) - DotProduct ((&m[8]), (&m[12]));
}

void R_Transform (float *m, float *out, const float *in)
{
	// copy off "in" so that we can use the same pointer for out and in
	float in2[3] = {in[0], in[1], in[2]};

	out[0] = in2[0] * m[0] + in2[1] * m[4] + in2[2] * m[ 8] + m[12];
	out[1] = in2[0] * m[1] + in2[1] * m[5] + in2[2] * m[ 9] + m[13];
	out[2] = in2[0] * m[2] + in2[1] * m[6] + in2[2] * m[10] + m[14];
}

void R_LocalMatrixForBModel (float *localmatrix, float *origin, float *angles)
{
	// initial local matrix is translation on identity
	localmatrix[0] = 1; localmatrix[4] = 0; localmatrix[ 8] = 0; localmatrix[12] = origin[0];
	localmatrix[1] = 0; localmatrix[5] = 1; localmatrix[ 9] = 0; localmatrix[13] = origin[1];
	localmatrix[2] = 0; localmatrix[6] = 0; localmatrix[10] = 1; localmatrix[14] = origin[2];
	localmatrix[3] = 0; localmatrix[7] = 0; localmatrix[11] = 0; localmatrix[15] = 1;

	// check for rotation and add it in if necessary
	if (angles[0] || angles[1] || angles[2])
	{
		AngleVectors (angles, &localmatrix[0], &localmatrix[4], &localmatrix[8]);
		VectorScale (&localmatrix[4], -1, &localmatrix[4]);
	}
}
#endif


/*
=============
R_LightPoint -- johnfitz -- replaced entire function for lit support via lordhavoc
=============
*/
int R_LightPoint (vec3_t p)
{
#ifdef TRUE_LIGHTPOINT
	int i;
	vec3_t end, vec;
	lightpoint_args_t args;

	if (!cl.worldmodel->lightdata)
	{
		r_lightpoint_args.color[0] = r_lightpoint_args.color[1] = r_lightpoint_args.color[2] = 255;
		return 255;
	}

	// correct for full world model bounds
	end[0] = p[0];
	end[1] = p[1];
	end[2] = cl.worldmodel->mins[2] - 10.0f;

	args.color[0] = args.color[1] = args.color[2] = 0;
	args.model = cl.worldmodel;

	RecursiveLightPoint (&args, cl.worldmodel->nodes, p, end);

	// get dist for comparison
	VectorSubtract (args.spot, p, vec);
	args.dist = Length (vec);

	// should this be ents or visedicts?
	// we'll use visedicts on the assumption that if you can't see it then you can't pick up lighting from it either
	for (i = 0; i < cl_numvisedicts; i++)
	{
		entity_t *ent = cl_visedicts[i];
		float estart[3], eend[3];
		lightpoint_args_t entargs;
		float localmatrix[16];

		// only inline bmodels (ent->model == NULL is impossible if using cl_visedicts but may become possible if using cl.entities;
		// however, cl.entities will not include static ents or temp ents, which may be important.)
		if (ent->model->type != mod_brush) continue;
		if (ent->model->name[0] != '*') continue;

		// to do: this should only be done once and when the bmodel is added as a visedict might be a good time to do it.
		// if you do this (and you really should) then you can also replace the R_RotateForEntity call for drawing bmodels
		// with a glMultMatrixf, and use R_InverseTransform for calculating cl_dlights[k].transformed in R_DrawBrushModel
		R_LocalMatrixForBModel (localmatrix, ent->origin, ent->angles);

		// move start and end points into the entity's local space
		R_InverseTransform (localmatrix, estart, p);
		R_InverseTransform (localmatrix, eend, end);

		// set up the lightpoint args
		entargs.color[0] = entargs.color[1] = entargs.color[2] = 0;
		entargs.model = ent->model;

		// and run the recursive light point on it too
		if (RecursiveLightPoint (&entargs, ent->model->nodes + ent->model->hulls[0].firstclipnode, estart, eend))
		{
			// a bmodel under a valid world hit will hit here too so take the highest lightspot on all hits
			// move lightspot back to world space
			R_Transform (localmatrix, entargs.spot, entargs.spot);

			// get dist for comparison
			VectorSubtract (entargs.spot, p, vec);
			entargs.dist = Length (vec);

			// because this was a direct trace downwards there's no danger of incorrectly using a bmodel that is
			// nearer but off to one side; it wouldn't have hit at all if that was the case
			if (entargs.dist < args.dist)
			{
				// found a bmodel so copy it over
				memcpy (&args, &entargs, sizeof (lightpoint_args_t));
			}
		}
	}

	// finally copy back to global args
	memcpy (&r_lightpoint_args, &args, sizeof (lightpoint_args_t));

	return ((r_lightpoint_args.color[0] + r_lightpoint_args.color[1] + r_lightpoint_args.color[2]) * (1.0f / 3.0f));
#else
	vec3_t		end;

	if (!cl.worldmodel->lightdata)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		return 255;
	}

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 8192; //johnfitz -- was 2048

	lightcolor[0] = lightcolor[1] = lightcolor[2] = 0;
	RecursiveLightPoint (lightcolor, cl.worldmodel->nodes, p, end);
	return ((lightcolor[0] + lightcolor[1] + lightcolor[2]) * (1.0f / 3.0f));
#endif
}
