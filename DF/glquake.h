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
// disable data conversion warnings

// i was getting tired of this during my testing
// this just implements the simplistic (and incorrect) "one frame later" optimization so that it's not too intrusive on the code and can be clearly walled-off
// it is expected that a more sophisticated engine port would do it right
#define DYNAMIC_LIGHT_OPTIMIZATION

// perform software Quake-ish underwater warp.
#define UNDERWATER_WARP

#define HOST_TIMER_FIX

#define ALPHAMASK_TEXTURES

#define TXGAMMA

#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef DIRECT3D9_WRAPPER
#include <gl/gl.h>
#include <GL/glu.h>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#else
#include "d3d9_publicapi.h"
#include "d3d9_apiglue.h"
#endif

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

//johnfitz -- removed texture object stuff since they are standard in gl 1.1

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t glv;

extern	int glx, gly, glwidth, glheight;

#ifdef _WIN32
extern	PROC glArrayElementEXT;
extern	PROC glColorPointerEXT;
extern	PROC glTexturePointerEXT;
extern	PROC glVertexPointerEXT;
#endif

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle
#define	MAX_LBM_HEIGHT		480

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01


void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
texture_t *R_TextureAnimation (texture_t *base, int frame);

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned			width;
	unsigned			height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;


typedef struct
{
	pixel_t		*surfdat;	// destination for generated surface
	int			rowbytes;	// destination logical width in bytes
	msurface_t	*surf;		// description for surface to generate
	fixed8_t	lightadj[MAXLIGHTMAPS];
							// adjust for lightmap levels for dynamic lighting
	texture_t	*texture;	// corrected for animating textures
	int			surfmip;	// mipmapped ratio of surface texels / world pixels
	int			surfwidth;	// in mipmapped texels
	int			surfheight;	// in mipmapped texels
} drawsurf_t;


typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
// driver-usable fields
	vec3_t		org;
	float		color;
// drivers never touch the following fields
	struct particle_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	ptype_t		type;
} particle_t;


//====================================================

extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	qboolean	envmap;

extern	cvar_t	r_norefresh;
extern	cvar_t	r_drawentities;
extern	cvar_t	r_drawworld;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_speeds;
extern	cvar_t	r_waterwarp;
extern	cvar_t	r_fullbright;
extern	cvar_t	r_lightmap;
extern	cvar_t	r_shadows;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;

extern	cvar_t	gl_clear;
extern	cvar_t	gl_cull;
extern	cvar_t	gl_smoothmodels;
extern	cvar_t	gl_affinemodels;
extern	cvar_t	gl_polyblend;
extern	cvar_t	gl_flashblend;
extern	cvar_t	gl_nocolors;

extern	cvar_t	gl_max_size;
extern	cvar_t	gl_playermip;

extern	float	r_world_matrix[16];

extern	const char *gl_vendor;
extern	const char *gl_renderer;
extern	const char *gl_version;
extern	const char *gl_extensions;

void R_TranslatePlayerSkin (int playernum);
void R_TranslateNewPlayerSkin (int playernum); //johnfitz -- this handles cases when the actual texture changes

// Multitexture
#define    TEXTURE0_SGIS				0x835E
#define    TEXTURE1_SGIS				0x835F

#ifndef _WIN32
#define APIENTRY /* */
#endif

//johnfitz -- modified multitexture support
typedef void (APIENTRY *SELECTTEXFUNC) (GLenum);
typedef void (APIENTRY *MTEXCOORDFUNC) (GLenum, GLfloat, GLfloat);
extern MTEXCOORDFUNC GL_MTexCoord2fFunc;
extern SELECTTEXFUNC GL_SelectTextureFunc;
#define	GL_TEXTURE0_ARB	0x84C0
#define	GL_TEXTURE1_ARB	0x84C1
extern GLenum TEXTURE0, TEXTURE1;
//johnfitz

//johnfitz -- anisotropic filtering
#define	GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define	GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
//johnfitz

//johnfitz -- polygon offset
#define OFFSET_BMODEL 1
#define OFFSET_NONE 0
#define OFFSET_DECAL -1
#define OFFSET_FOG -2
#define OFFSET_SHOWTRIS -3
void GL_PolygonOffset (int);
//johnfitz

//johnfitz -- GL_EXT_texture_env_combine
//the values for GL_ARB_ are identical
#define GL_COMBINE_EXT			0x8570
#define GL_COMBINE_RGB_EXT		0x8571
#define GL_COMBINE_ALPHA_EXT	0x8572
#define GL_RGB_SCALE_EXT		0x8573
#define GL_CONSTANT_EXT			0x8576
#define GL_PRIMARY_COLOR_EXT	0x8577
#define GL_PREVIOUS_EXT			0x8578
#define GL_SOURCE0_RGB_EXT		0x8580
#define GL_SOURCE1_RGB_EXT		0x8581
#define GL_SOURCE0_ALPHA_EXT	0x8588
#define GL_SOURCE1_ALPHA_EXT	0x8589
extern qboolean gl_texture_env_combine;
//johnfitz

#ifdef TEXTURE_NPO2
extern qboolean gl_texture_npo2;
#endif

extern qboolean gl_texture_env_add; //johnfitz -- for GL_EXT_texture_env_add

extern qboolean isIntelVideo; //johnfitz -- intel video workarounds from Baker

//johnfitz -- rendering statistics
extern int rs_brushpolys, rs_aliaspolys, rs_skypolys, rs_particles, rs_fogpolys;
extern int rs_dynamiclightmaps, rs_brushpasses, rs_aliaspasses, rs_skypasses;
extern float rs_megatexels;
//johnfitz

//johnfitz -- track developer statistics that vary every frame
extern cvar_t devstats;
typedef struct {
	int		packetsize;
	int		edicts;
	int		visedicts;
	int		efrags;
	int		tempents;
	int		beams;
	int		dlights;
} devstats_t;
devstats_t dev_stats, dev_peakstats;
//johnfitz

//ohnfitz -- reduce overflow warning spam
typedef struct {
	double	packetsize;
	double	efrags;
	double	beams;
} overflowtimes_t;
overflowtimes_t dev_overflows; //this stores the last time overflow messages were displayed, not the last time overflows occured
#define CONSOLE_RESPAM_TIME 3 // seconds between repeated warning messages
//johnfitz

//johnfitz -- moved here from r_brush.c
#define MAX_LIGHTMAPS 256 //johnfitz -- was 64
gltexture_t *lightmap_textures[MAX_LIGHTMAPS]; //johnfitz -- changed to an array
//johnfitz

int gl_warpimagesize; //johnfitz -- for water warp

qboolean r_drawflat_cheatsafe, r_fullbright_cheatsafe, r_lightmap_cheatsafe, r_drawworld_cheatsafe; //johnfitz

//johnfitz -- fog functions called from outside gl_fog.c
void Fog_ParseServerMessage (void);
float *Fog_GetColor (void);
float Fog_GetDensity (void);
void Fog_EnableGFog (void);
void Fog_DisableGFog (void);
void Fog_StartAdditive (void);
void Fog_StopAdditive (void);
void Fog_SetupFrame (void);
void Fog_NewMap (void);
void Fog_Init (void);
//johnfitz


#ifdef DYNAMIC_LIGHT_OPTIMIZATION
void R_UploadLightmaps (void);
#endif

#ifdef UNDERWATER_WARP
extern cvar_t r_waterwarp_cycle;
extern cvar_t r_waterwarp_amp;
extern cvar_t r_waterwarp_downscale;

void R_InitUnderwaterWarpTexture (void);
#endif

#ifdef TRUE_LIGHTPOINT
typedef struct lightpoint_args_s
{
	mplane_t		*plane;
	vec3_t			spot;
	vec3_t			color; //johnfitz -- lit support via lordhavoc
	msurface_t		*surf;
	float			dist; // trivial to recreate
	model_t			*model;
} lightpoint_args_t;
#endif
