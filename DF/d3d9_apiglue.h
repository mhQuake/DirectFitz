/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

// API "glue" between the engine and the wrapper; this is just a simplistic "get it working" layer which implements
// what's necessary to interface the two while retaining Baker's naming conventions.  a proper implementation would
// set functions pointers or whatever, and you don't need to include this file in your project.

// OpenGL 1.1
#define glAccum d3d9mh_glAccum
#define glAlphaFunc d3d9mh_glAlphaFunc
#define glAreTexturesResident d3d9mh_glAreTexturesResident
#define glArrayElement d3d9mh_glArrayElement
#define glBegin d3d9mh_glBegin
#define glBindTexture d3d9mh_glBindTexture
#define glBitmap d3d9mh_glBitmap
#define glBlendFunc d3d9mh_glBlendFunc
#define glCallList d3d9mh_glCallList
#define glCallLists d3d9mh_glCallLists
#define glClear d3d9mh_glClear
#define glClearAccum d3d9mh_glClearAccum
#define glClearColor d3d9mh_glClearColor
#define glClearDepth d3d9mh_glClearDepth
#define glClearIndex d3d9mh_glClearIndex
#define glClearStencil d3d9mh_glClearStencil
#define glClipPlane d3d9mh_glClipPlane
#define glColor3b d3d9mh_glColor3b
#define glColor3bv d3d9mh_glColor3bv
#define glColor3d d3d9mh_glColor3d
#define glColor3dv d3d9mh_glColor3dv
#define glColor3f d3d9mh_glColor3f
#define glColor3fv d3d9mh_glColor3fv
#define glColor3i d3d9mh_glColor3i
#define glColor3iv d3d9mh_glColor3iv
#define glColor3s d3d9mh_glColor3s
#define glColor3sv d3d9mh_glColor3sv
#define glColor3ub d3d9mh_glColor3ub
#define glColor3ubv d3d9mh_glColor3ubv
#define glColor3ui d3d9mh_glColor3ui
#define glColor3uiv d3d9mh_glColor3uiv
#define glColor3us d3d9mh_glColor3us
#define glColor3usv d3d9mh_glColor3usv
#define glColor4b d3d9mh_glColor4b
#define glColor4bv d3d9mh_glColor4bv
#define glColor4d d3d9mh_glColor4d
#define glColor4dv d3d9mh_glColor4dv
#define glColor4f d3d9mh_glColor4f
#define glColor4fv d3d9mh_glColor4fv
#define glColor4i d3d9mh_glColor4i
#define glColor4iv d3d9mh_glColor4iv
#define glColor4s d3d9mh_glColor4s
#define glColor4sv d3d9mh_glColor4sv
#define glColor4ub d3d9mh_glColor4ub
#define glColor4ubv d3d9mh_glColor4ubv
#define glColor4ui d3d9mh_glColor4ui
#define glColor4uiv d3d9mh_glColor4uiv
#define glColor4us d3d9mh_glColor4us
#define glColor4usv d3d9mh_glColor4usv
#define glColorMask d3d9mh_glColorMask
#define glColorMaterial d3d9mh_glColorMaterial
#define glColorPointer d3d9mh_glColorPointer
#define glCopyPixels d3d9mh_glCopyPixels
#define glCopyTexImage1D d3d9mh_glCopyTexImage1D
#define glCopyTexImage2D d3d9mh_glCopyTexImage2D
#define glCopyTexSubImage1D d3d9mh_glCopyTexSubImage1D
#define glCopyTexSubImage2D d3d9mh_glCopyTexSubImage2D
#define glCullFace d3d9mh_glCullFace
#define glDeleteLists d3d9mh_glDeleteLists
#define glDeleteTextures d3d9mh_glDeleteTextures
#define glDepthFunc d3d9mh_glDepthFunc
#define glDepthMask d3d9mh_glDepthMask
#define glDepthRange d3d9mh_glDepthRange
#define glDisable d3d9mh_glDisable
#define glDisableClientState d3d9mh_glDisableClientState
#define glDrawArrays d3d9mh_glDrawArrays
#define glDrawBuffer d3d9mh_glDrawBuffer
#define glDrawElements d3d9mh_glDrawElements
#define glDrawPixels d3d9mh_glDrawPixels
#define glEdgeFlag d3d9mh_glEdgeFlag
#define glEdgeFlagPointer d3d9mh_glEdgeFlagPointer
#define glEdgeFlagv d3d9mh_glEdgeFlagv
#define glEnable d3d9mh_glEnable
#define glEnableClientState d3d9mh_glEnableClientState
#define glEnd d3d9mh_glEnd
#define glEndList d3d9mh_glEndList
#define glEvalCoord1d d3d9mh_glEvalCoord1d
#define glEvalCoord1dv d3d9mh_glEvalCoord1dv
#define glEvalCoord1f d3d9mh_glEvalCoord1f
#define glEvalCoord1fv d3d9mh_glEvalCoord1fv
#define glEvalCoord2d d3d9mh_glEvalCoord2d
#define glEvalCoord2dv d3d9mh_glEvalCoord2dv
#define glEvalCoord2f d3d9mh_glEvalCoord2f
#define glEvalCoord2fv d3d9mh_glEvalCoord2fv
#define glEvalMesh1 d3d9mh_glEvalMesh1
#define glEvalMesh2 d3d9mh_glEvalMesh2
#define glEvalPoint1 d3d9mh_glEvalPoint1
#define glEvalPoint2 d3d9mh_glEvalPoint2
#define glFeedbackBuffer d3d9mh_glFeedbackBuffer
#define glFinish d3d9mh_glFinish
#define glFlush d3d9mh_glFlush
#define glFogf d3d9mh_glFogf
#define glFogfv d3d9mh_glFogfv
#define glFogi d3d9mh_glFogi
#define glFogiv d3d9mh_glFogiv
#define glFrontFace d3d9mh_glFrontFace
#define glFrustum d3d9mh_glFrustum
#define glGenLists d3d9mh_glGenLists
#define glGenTextures d3d9mh_glGenTextures
#define glGetBooleanv d3d9mh_glGetBooleanv
#define glGetClipPlane d3d9mh_glGetClipPlane
#define glGetDoublev d3d9mh_glGetDoublev
#define glGetError d3d9mh_glGetError
#define glGetFloatv d3d9mh_glGetFloatv
#define glGetIntegerv d3d9mh_glGetIntegerv
#define glGetLightfv d3d9mh_glGetLightfv
#define glGetLightiv d3d9mh_glGetLightiv
#define glGetMapdv d3d9mh_glGetMapdv
#define glGetMapfv d3d9mh_glGetMapfv
#define glGetMapiv d3d9mh_glGetMapiv
#define glGetMaterialfv d3d9mh_glGetMaterialfv
#define glGetMaterialiv d3d9mh_glGetMaterialiv
#define glGetPixelMapfv d3d9mh_glGetPixelMapfv
#define glGetPixelMapuiv d3d9mh_glGetPixelMapuiv
#define glGetPixelMapusv d3d9mh_glGetPixelMapusv
#define glGetPointerv d3d9mh_glGetPointerv
#define glGetPolygonStipple d3d9mh_glGetPolygonStipple
#define glGetString d3d9mh_glGetString
#define glGetTexEnvfv d3d9mh_glGetTexEnvfv
#define glGetTexEnviv d3d9mh_glGetTexEnviv
#define glGetTexGendv d3d9mh_glGetTexGendv
#define glGetTexGenfv d3d9mh_glGetTexGenfv
#define glGetTexGeniv d3d9mh_glGetTexGeniv
#define glGetTexImage d3d9mh_glGetTexImage
#define glGetTexLevelParameterfv d3d9mh_glGetTexLevelParameterfv
#define glGetTexLevelParameteriv d3d9mh_glGetTexLevelParameteriv
#define glGetTexParameterfv d3d9mh_glGetTexParameterfv
#define glGetTexParameteriv d3d9mh_glGetTexParameteriv
#define glHint d3d9mh_glHint
#define glIndexMask d3d9mh_glIndexMask
#define glIndexPointer d3d9mh_glIndexPointer
#define glIndexd d3d9mh_glIndexd
#define glIndexdv d3d9mh_glIndexdv
#define glIndexf d3d9mh_glIndexf
#define glIndexfv d3d9mh_glIndexfv
#define glIndexi d3d9mh_glIndexi
#define glIndexiv d3d9mh_glIndexiv
#define glIndexs d3d9mh_glIndexs
#define glIndexsv d3d9mh_glIndexsv
#define glIndexub d3d9mh_glIndexub
#define glIndexubv d3d9mh_glIndexubv
#define glInitNames d3d9mh_glInitNames
#define glInterleavedArrays d3d9mh_glInterleavedArrays
#define glIsEnabled d3d9mh_glIsEnabled
#define glIsList d3d9mh_glIsList
#define glIsTexture d3d9mh_glIsTexture
#define glLightModelf d3d9mh_glLightModelf
#define glLightModelfv d3d9mh_glLightModelfv
#define glLightModeli d3d9mh_glLightModeli
#define glLightModeliv d3d9mh_glLightModeliv
#define glLightf d3d9mh_glLightf
#define glLightfv d3d9mh_glLightfv
#define glLighti d3d9mh_glLighti
#define glLightiv d3d9mh_glLightiv
#define glLineStipple d3d9mh_glLineStipple
#define glLineWidth d3d9mh_glLineWidth
#define glListBase d3d9mh_glListBase
#define glLoadIdentity d3d9mh_glLoadIdentity
#define glLoadMatrixd d3d9mh_glLoadMatrixd
#define glLoadMatrixf d3d9mh_glLoadMatrixf
#define glLoadName d3d9mh_glLoadName
#define glLogicOp d3d9mh_glLogicOp
#define glMap1d d3d9mh_glMap1d
#define glMap1f d3d9mh_glMap1f
#define glMap2d d3d9mh_glMap2d
#define glMap2f d3d9mh_glMap2f
#define glMapGrid1d d3d9mh_glMapGrid1d
#define glMapGrid1f d3d9mh_glMapGrid1f
#define glMapGrid2d d3d9mh_glMapGrid2d
#define glMapGrid2f d3d9mh_glMapGrid2f
#define glMaterialf d3d9mh_glMaterialf
#define glMaterialfv d3d9mh_glMaterialfv
#define glMateriali d3d9mh_glMateriali
#define glMaterialiv d3d9mh_glMaterialiv
#define glMatrixMode d3d9mh_glMatrixMode
#define glMultMatrixd d3d9mh_glMultMatrixd
#define glMultMatrixf d3d9mh_glMultMatrixf
#define glNewList d3d9mh_glNewList
#define glNormal3b d3d9mh_glNormal3b
#define glNormal3bv d3d9mh_glNormal3bv
#define glNormal3d d3d9mh_glNormal3d
#define glNormal3dv d3d9mh_glNormal3dv
#define glNormal3f d3d9mh_glNormal3f
#define glNormal3fv d3d9mh_glNormal3fv
#define glNormal3i d3d9mh_glNormal3i
#define glNormal3iv d3d9mh_glNormal3iv
#define glNormal3s d3d9mh_glNormal3s
#define glNormal3sv d3d9mh_glNormal3sv
#define glNormalPointer d3d9mh_glNormalPointer
#define glOrtho d3d9mh_glOrtho
#define glPassThrough d3d9mh_glPassThrough
#define glPixelMapfv d3d9mh_glPixelMapfv
#define glPixelMapuiv d3d9mh_glPixelMapuiv
#define glPixelMapusv d3d9mh_glPixelMapusv
#define glPixelStoref d3d9mh_glPixelStoref
#define glPixelStorei d3d9mh_glPixelStorei
#define glPixelTransferf d3d9mh_glPixelTransferf
#define glPixelTransferi d3d9mh_glPixelTransferi
#define glPixelZoom d3d9mh_glPixelZoom
#define glPointSize d3d9mh_glPointSize
#define glPolygonMode d3d9mh_glPolygonMode
#define glPolygonOffset d3d9mh_glPolygonOffset
#define glPolygonStipple d3d9mh_glPolygonStipple
#define glPopAttrib d3d9mh_glPopAttrib
#define glPopClientAttrib d3d9mh_glPopClientAttrib
#define glPopMatrix d3d9mh_glPopMatrix
#define glPopName d3d9mh_glPopName
#define glPrioritizeTextures d3d9mh_glPrioritizeTextures
#define glPushAttrib d3d9mh_glPushAttrib
#define glPushClientAttrib d3d9mh_glPushClientAttrib
#define glPushMatrix d3d9mh_glPushMatrix
#define glPushName d3d9mh_glPushName
#define glRasterPos2d d3d9mh_glRasterPos2d
#define glRasterPos2dv d3d9mh_glRasterPos2dv
#define glRasterPos2f d3d9mh_glRasterPos2f
#define glRasterPos2fv d3d9mh_glRasterPos2fv
#define glRasterPos2i d3d9mh_glRasterPos2i
#define glRasterPos2iv d3d9mh_glRasterPos2iv
#define glRasterPos2s d3d9mh_glRasterPos2s
#define glRasterPos2sv d3d9mh_glRasterPos2sv
#define glRasterPos3d d3d9mh_glRasterPos3d
#define glRasterPos3dv d3d9mh_glRasterPos3dv
#define glRasterPos3f d3d9mh_glRasterPos3f
#define glRasterPos3fv d3d9mh_glRasterPos3fv
#define glRasterPos3i d3d9mh_glRasterPos3i
#define glRasterPos3iv d3d9mh_glRasterPos3iv
#define glRasterPos3s d3d9mh_glRasterPos3s
#define glRasterPos3sv d3d9mh_glRasterPos3sv
#define glRasterPos4d d3d9mh_glRasterPos4d
#define glRasterPos4dv d3d9mh_glRasterPos4dv
#define glRasterPos4f d3d9mh_glRasterPos4f
#define glRasterPos4fv d3d9mh_glRasterPos4fv
#define glRasterPos4i d3d9mh_glRasterPos4i
#define glRasterPos4iv d3d9mh_glRasterPos4iv
#define glRasterPos4s d3d9mh_glRasterPos4s
#define glRasterPos4sv d3d9mh_glRasterPos4sv
#define glReadBuffer d3d9mh_glReadBuffer
#define glReadPixels d3d9mh_glReadPixels
#define glRectd d3d9mh_glRectd
#define glRectdv d3d9mh_glRectdv
#define glRectf d3d9mh_glRectf
#define glRectfv d3d9mh_glRectfv
#define glRecti d3d9mh_glRecti
#define glRectiv d3d9mh_glRectiv
#define glRects d3d9mh_glRects
#define glRectsv d3d9mh_glRectsv
#define glRenderMode d3d9mh_glRenderMode
#define glRotated d3d9mh_glRotated
#define glRotatef d3d9mh_glRotatef
#define glScaled d3d9mh_glScaled
#define glScalef d3d9mh_glScalef
#define glScissor d3d9mh_glScissor
#define glSelectBuffer d3d9mh_glSelectBuffer
#define glShadeModel d3d9mh_glShadeModel
#define glStencilFunc d3d9mh_glStencilFunc
#define glStencilMask d3d9mh_glStencilMask
#define glStencilOp d3d9mh_glStencilOp
#define glTexCoord1d d3d9mh_glTexCoord1d
#define glTexCoord1dv d3d9mh_glTexCoord1dv
#define glTexCoord1f d3d9mh_glTexCoord1f
#define glTexCoord1fv d3d9mh_glTexCoord1fv
#define glTexCoord1i d3d9mh_glTexCoord1i
#define glTexCoord1iv d3d9mh_glTexCoord1iv
#define glTexCoord1s d3d9mh_glTexCoord1s
#define glTexCoord1sv d3d9mh_glTexCoord1sv
#define glTexCoord2d d3d9mh_glTexCoord2d
#define glTexCoord2dv d3d9mh_glTexCoord2dv
#define glTexCoord2f d3d9mh_glTexCoord2f
#define glTexCoord2fv d3d9mh_glTexCoord2fv
#define glTexCoord2i d3d9mh_glTexCoord2i
#define glTexCoord2iv d3d9mh_glTexCoord2iv
#define glTexCoord2s d3d9mh_glTexCoord2s
#define glTexCoord2sv d3d9mh_glTexCoord2sv
#define glTexCoord3d d3d9mh_glTexCoord3d
#define glTexCoord3dv d3d9mh_glTexCoord3dv
#define glTexCoord3f d3d9mh_glTexCoord3f
#define glTexCoord3fv d3d9mh_glTexCoord3fv
#define glTexCoord3i d3d9mh_glTexCoord3i
#define glTexCoord3iv d3d9mh_glTexCoord3iv
#define glTexCoord3s d3d9mh_glTexCoord3s
#define glTexCoord3sv d3d9mh_glTexCoord3sv
#define glTexCoord4d d3d9mh_glTexCoord4d
#define glTexCoord4dv d3d9mh_glTexCoord4dv
#define glTexCoord4f d3d9mh_glTexCoord4f
#define glTexCoord4fv d3d9mh_glTexCoord4fv
#define glTexCoord4i d3d9mh_glTexCoord4i
#define glTexCoord4iv d3d9mh_glTexCoord4iv
#define glTexCoord4s d3d9mh_glTexCoord4s
#define glTexCoord4sv d3d9mh_glTexCoord4sv
#define glTexCoordPointer d3d9mh_glTexCoordPointer
#define glTexEnvf d3d9mh_glTexEnvf
#define glTexEnvfv d3d9mh_glTexEnvfv
#define glTexEnvi d3d9mh_glTexEnvi
#define glTexEnviv d3d9mh_glTexEnviv
#define glTexGend d3d9mh_glTexGend
#define glTexGendv d3d9mh_glTexGendv
#define glTexGenf d3d9mh_glTexGenf
#define glTexGenfv d3d9mh_glTexGenfv
#define glTexGeni d3d9mh_glTexGeni
#define glTexGeniv d3d9mh_glTexGeniv
#define glTexImage1D d3d9mh_glTexImage1D
#define glTexImage2D d3d9mh_glTexImage2D
#define glTexParameterf d3d9mh_glTexParameterf
#define glTexParameterfv d3d9mh_glTexParameterfv
#define glTexParameteri d3d9mh_glTexParameteri
#define glTexParameteriv d3d9mh_glTexParameteriv
#define glTexSubImage1D d3d9mh_glTexSubImage1D
#define glTexSubImage2D d3d9mh_glTexSubImage2D
#define glTranslated d3d9mh_glTranslated
#define glTranslatef d3d9mh_glTranslatef
#define glVertex2d d3d9mh_glVertex2d
#define glVertex2dv d3d9mh_glVertex2dv
#define glVertex2f d3d9mh_glVertex2f
#define glVertex2fv d3d9mh_glVertex2fv
#define glVertex2i d3d9mh_glVertex2i
#define glVertex2iv d3d9mh_glVertex2iv
#define glVertex2s d3d9mh_glVertex2s
#define glVertex2sv d3d9mh_glVertex2sv
#define glVertex3d d3d9mh_glVertex3d
#define glVertex3dv d3d9mh_glVertex3dv
#define glVertex3f d3d9mh_glVertex3f
#define glVertex3fv d3d9mh_glVertex3fv
#define glVertex3i d3d9mh_glVertex3i
#define glVertex3iv d3d9mh_glVertex3iv
#define glVertex3s d3d9mh_glVertex3s
#define glVertex3sv d3d9mh_glVertex3sv
#define glVertex4d d3d9mh_glVertex4d
#define glVertex4dv d3d9mh_glVertex4dv
#define glVertex4f d3d9mh_glVertex4f
#define glVertex4fv d3d9mh_glVertex4fv
#define glVertex4i d3d9mh_glVertex4i
#define glVertex4iv d3d9mh_glVertex4iv
#define glVertex4s d3d9mh_glVertex4s
#define glVertex4sv d3d9mh_glVertex4sv
#define glVertexPointer d3d9mh_glVertexPointer
#define glViewport d3d9mh_glViewport

// WGL
#define wglCreateContext Direct3D9_wglCreateContext
#define wglDeleteContext Direct3D9_wglDeleteContext
#define wglGetCurrentContext Direct3D9_wglGetCurrentContext
#define wglGetCurrentDC Direct3D9_wglGetCurrentDC
#define wglMakeCurrent Direct3D9_wglMakeCurrent
#define wglGetProcAddress Direct3D9_wglGetProcAddress
