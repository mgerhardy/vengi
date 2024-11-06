/* WARNING: This file was automatically generated */
/* Do not edit. */

#include "flextGL.h"
#include <SDL3/SDL.h>


#ifdef __cplusplus
extern "C" {
#endif


void flextLoadOpenGLFunctions(void);

/**
 * \return -1 on error, 0 on success
 * \sa SDL_GetError
 */
int flextInit(void)
{
    flextLoadOpenGLFunctions();

    /* --- Check for extensions --- */

    if (SDL_GL_ExtensionSupported("GL_ARB_debug_output")) {
        FLEXT_ARB_debug_output = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_direct_state_access")) {
        FLEXT_ARB_direct_state_access = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_buffer_storage")) {
        FLEXT_ARB_buffer_storage = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_multi_draw_indirect")) {
        FLEXT_ARB_multi_draw_indirect = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_draw_indirect")) {
        FLEXT_ARB_draw_indirect = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_instanced_arrays")) {
        FLEXT_ARB_instanced_arrays = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_compute_shader")) {
        FLEXT_ARB_compute_shader = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_shader_image_load_store")) {
        FLEXT_ARB_shader_image_load_store = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_transform_feedback2")) {
        FLEXT_ARB_transform_feedback2 = GL_TRUE;
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_shader_storage_buffer_object")) {
        FLEXT_ARB_shader_storage_buffer_object = GL_TRUE;
    }


    return 0;
}

void flextLoadOpenGLFunctions(void)
{
    /* --- Function pointer loading --- */

    /* GL_ARB_buffer_storage */

    glpfBufferStorage = (PFNGLBUFFERSTORAGE_PROC*)SDL_GL_GetProcAddress("glBufferStorage");

    /* GL_ARB_compute_shader */

    glpfDispatchCompute = (PFNGLDISPATCHCOMPUTE_PROC*)SDL_GL_GetProcAddress("glDispatchCompute");
    glpfDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECT_PROC*)SDL_GL_GetProcAddress("glDispatchComputeIndirect");

    /* GL_ARB_debug_output */

    glpfDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARB_PROC*)SDL_GL_GetProcAddress("glDebugMessageCallbackARB");
    glpfDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARB_PROC*)SDL_GL_GetProcAddress("glDebugMessageControlARB");
    glpfDebugMessageInsertARB = (PFNGLDEBUGMESSAGEINSERTARB_PROC*)SDL_GL_GetProcAddress("glDebugMessageInsertARB");
    glpfGetDebugMessageLogARB = (PFNGLGETDEBUGMESSAGELOGARB_PROC*)SDL_GL_GetProcAddress("glGetDebugMessageLogARB");

    /* GL_ARB_direct_state_access */

    glpfBindTextureUnit = (PFNGLBINDTEXTUREUNIT_PROC*)SDL_GL_GetProcAddress("glBindTextureUnit");
    glpfBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFER_PROC*)SDL_GL_GetProcAddress("glBlitNamedFramebuffer");
    glpfCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUS_PROC*)SDL_GL_GetProcAddress("glCheckNamedFramebufferStatus");
    glpfClearNamedBufferData = (PFNGLCLEARNAMEDBUFFERDATA_PROC*)SDL_GL_GetProcAddress("glClearNamedBufferData");
    glpfClearNamedBufferSubData = (PFNGLCLEARNAMEDBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glClearNamedBufferSubData");
    glpfClearNamedFramebufferfi = (PFNGLCLEARNAMEDFRAMEBUFFERFI_PROC*)SDL_GL_GetProcAddress("glClearNamedFramebufferfi");
    glpfClearNamedFramebufferfv = (PFNGLCLEARNAMEDFRAMEBUFFERFV_PROC*)SDL_GL_GetProcAddress("glClearNamedFramebufferfv");
    glpfClearNamedFramebufferiv = (PFNGLCLEARNAMEDFRAMEBUFFERIV_PROC*)SDL_GL_GetProcAddress("glClearNamedFramebufferiv");
    glpfClearNamedFramebufferuiv = (PFNGLCLEARNAMEDFRAMEBUFFERUIV_PROC*)SDL_GL_GetProcAddress("glClearNamedFramebufferuiv");
    glpfCompressedTextureSubImage1D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCompressedTextureSubImage1D");
    glpfCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCompressedTextureSubImage2D");
    glpfCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glCompressedTextureSubImage3D");
    glpfCopyNamedBufferSubData = (PFNGLCOPYNAMEDBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glCopyNamedBufferSubData");
    glpfCopyTextureSubImage1D = (PFNGLCOPYTEXTURESUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCopyTextureSubImage1D");
    glpfCopyTextureSubImage2D = (PFNGLCOPYTEXTURESUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCopyTextureSubImage2D");
    glpfCopyTextureSubImage3D = (PFNGLCOPYTEXTURESUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glCopyTextureSubImage3D");
    glpfCreateBuffers = (PFNGLCREATEBUFFERS_PROC*)SDL_GL_GetProcAddress("glCreateBuffers");
    glpfCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERS_PROC*)SDL_GL_GetProcAddress("glCreateFramebuffers");
    glpfCreateProgramPipelines = (PFNGLCREATEPROGRAMPIPELINES_PROC*)SDL_GL_GetProcAddress("glCreateProgramPipelines");
    glpfCreateQueries = (PFNGLCREATEQUERIES_PROC*)SDL_GL_GetProcAddress("glCreateQueries");
    glpfCreateRenderbuffers = (PFNGLCREATERENDERBUFFERS_PROC*)SDL_GL_GetProcAddress("glCreateRenderbuffers");
    glpfCreateSamplers = (PFNGLCREATESAMPLERS_PROC*)SDL_GL_GetProcAddress("glCreateSamplers");
    glpfCreateTextures = (PFNGLCREATETEXTURES_PROC*)SDL_GL_GetProcAddress("glCreateTextures");
    glpfCreateTransformFeedbacks = (PFNGLCREATETRANSFORMFEEDBACKS_PROC*)SDL_GL_GetProcAddress("glCreateTransformFeedbacks");
    glpfCreateVertexArrays = (PFNGLCREATEVERTEXARRAYS_PROC*)SDL_GL_GetProcAddress("glCreateVertexArrays");
    glpfDisableVertexArrayAttrib = (PFNGLDISABLEVERTEXARRAYATTRIB_PROC*)SDL_GL_GetProcAddress("glDisableVertexArrayAttrib");
    glpfEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIB_PROC*)SDL_GL_GetProcAddress("glEnableVertexArrayAttrib");
    glpfFlushMappedNamedBufferRange = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glFlushMappedNamedBufferRange");
    glpfGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAP_PROC*)SDL_GL_GetProcAddress("glGenerateTextureMipmap");
    glpfGetCompressedTextureImage = (PFNGLGETCOMPRESSEDTEXTUREIMAGE_PROC*)SDL_GL_GetProcAddress("glGetCompressedTextureImage");
    glpfGetNamedBufferParameteri64v = (PFNGLGETNAMEDBUFFERPARAMETERI64V_PROC*)SDL_GL_GetProcAddress("glGetNamedBufferParameteri64v");
    glpfGetNamedBufferParameteriv = (PFNGLGETNAMEDBUFFERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetNamedBufferParameteriv");
    glpfGetNamedBufferPointerv = (PFNGLGETNAMEDBUFFERPOINTERV_PROC*)SDL_GL_GetProcAddress("glGetNamedBufferPointerv");
    glpfGetNamedBufferSubData = (PFNGLGETNAMEDBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glGetNamedBufferSubData");
    glpfGetNamedFramebufferAttachmentParameteriv = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetNamedFramebufferAttachmentParameteriv");
    glpfGetNamedFramebufferParameteriv = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetNamedFramebufferParameteriv");
    glpfGetNamedRenderbufferParameteriv = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetNamedRenderbufferParameteriv");
    glpfGetQueryBufferObjecti64v = (PFNGLGETQUERYBUFFEROBJECTI64V_PROC*)SDL_GL_GetProcAddress("glGetQueryBufferObjecti64v");
    glpfGetQueryBufferObjectiv = (PFNGLGETQUERYBUFFEROBJECTIV_PROC*)SDL_GL_GetProcAddress("glGetQueryBufferObjectiv");
    glpfGetQueryBufferObjectui64v = (PFNGLGETQUERYBUFFEROBJECTUI64V_PROC*)SDL_GL_GetProcAddress("glGetQueryBufferObjectui64v");
    glpfGetQueryBufferObjectuiv = (PFNGLGETQUERYBUFFEROBJECTUIV_PROC*)SDL_GL_GetProcAddress("glGetQueryBufferObjectuiv");
    glpfGetTextureImage = (PFNGLGETTEXTUREIMAGE_PROC*)SDL_GL_GetProcAddress("glGetTextureImage");
    glpfGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glGetTextureLevelParameterfv");
    glpfGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetTextureLevelParameteriv");
    glpfGetTextureParameterIiv = (PFNGLGETTEXTUREPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glGetTextureParameterIiv");
    glpfGetTextureParameterIuiv = (PFNGLGETTEXTUREPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glGetTextureParameterIuiv");
    glpfGetTextureParameterfv = (PFNGLGETTEXTUREPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glGetTextureParameterfv");
    glpfGetTextureParameteriv = (PFNGLGETTEXTUREPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetTextureParameteriv");
    glpfGetTransformFeedbacki64_v = (PFNGLGETTRANSFORMFEEDBACKI64_V_PROC*)SDL_GL_GetProcAddress("glGetTransformFeedbacki64_v");
    glpfGetTransformFeedbacki_v = (PFNGLGETTRANSFORMFEEDBACKI_V_PROC*)SDL_GL_GetProcAddress("glGetTransformFeedbacki_v");
    glpfGetTransformFeedbackiv = (PFNGLGETTRANSFORMFEEDBACKIV_PROC*)SDL_GL_GetProcAddress("glGetTransformFeedbackiv");
    glpfGetVertexArrayIndexed64iv = (PFNGLGETVERTEXARRAYINDEXED64IV_PROC*)SDL_GL_GetProcAddress("glGetVertexArrayIndexed64iv");
    glpfGetVertexArrayIndexediv = (PFNGLGETVERTEXARRAYINDEXEDIV_PROC*)SDL_GL_GetProcAddress("glGetVertexArrayIndexediv");
    glpfGetVertexArrayiv = (PFNGLGETVERTEXARRAYIV_PROC*)SDL_GL_GetProcAddress("glGetVertexArrayiv");
    glpfInvalidateNamedFramebufferData = (PFNGLINVALIDATENAMEDFRAMEBUFFERDATA_PROC*)SDL_GL_GetProcAddress("glInvalidateNamedFramebufferData");
    glpfInvalidateNamedFramebufferSubData = (PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glInvalidateNamedFramebufferSubData");
    glpfMapNamedBuffer = (PFNGLMAPNAMEDBUFFER_PROC*)SDL_GL_GetProcAddress("glMapNamedBuffer");
    glpfMapNamedBufferRange = (PFNGLMAPNAMEDBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glMapNamedBufferRange");
    glpfNamedBufferData = (PFNGLNAMEDBUFFERDATA_PROC*)SDL_GL_GetProcAddress("glNamedBufferData");
    glpfNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGE_PROC*)SDL_GL_GetProcAddress("glNamedBufferStorage");
    glpfNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glNamedBufferSubData");
    glpfNamedFramebufferDrawBuffer = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFER_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferDrawBuffer");
    glpfNamedFramebufferDrawBuffers = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERS_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferDrawBuffers");
    glpfNamedFramebufferParameteri = (PFNGLNAMEDFRAMEBUFFERPARAMETERI_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferParameteri");
    glpfNamedFramebufferReadBuffer = (PFNGLNAMEDFRAMEBUFFERREADBUFFER_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferReadBuffer");
    glpfNamedFramebufferRenderbuffer = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFER_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferRenderbuffer");
    glpfNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTURE_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferTexture");
    glpfNamedFramebufferTextureLayer = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYER_PROC*)SDL_GL_GetProcAddress("glNamedFramebufferTextureLayer");
    glpfNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGE_PROC*)SDL_GL_GetProcAddress("glNamedRenderbufferStorage");
    glpfNamedRenderbufferStorageMultisample = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glNamedRenderbufferStorageMultisample");
    glpfTextureBuffer = (PFNGLTEXTUREBUFFER_PROC*)SDL_GL_GetProcAddress("glTextureBuffer");
    glpfTextureBufferRange = (PFNGLTEXTUREBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glTextureBufferRange");
    glpfTextureParameterIiv = (PFNGLTEXTUREPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glTextureParameterIiv");
    glpfTextureParameterIuiv = (PFNGLTEXTUREPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glTextureParameterIuiv");
    glpfTextureParameterf = (PFNGLTEXTUREPARAMETERF_PROC*)SDL_GL_GetProcAddress("glTextureParameterf");
    glpfTextureParameterfv = (PFNGLTEXTUREPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glTextureParameterfv");
    glpfTextureParameteri = (PFNGLTEXTUREPARAMETERI_PROC*)SDL_GL_GetProcAddress("glTextureParameteri");
    glpfTextureParameteriv = (PFNGLTEXTUREPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glTextureParameteriv");
    glpfTextureStorage1D = (PFNGLTEXTURESTORAGE1D_PROC*)SDL_GL_GetProcAddress("glTextureStorage1D");
    glpfTextureStorage2D = (PFNGLTEXTURESTORAGE2D_PROC*)SDL_GL_GetProcAddress("glTextureStorage2D");
    glpfTextureStorage2DMultisample = (PFNGLTEXTURESTORAGE2DMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glTextureStorage2DMultisample");
    glpfTextureStorage3D = (PFNGLTEXTURESTORAGE3D_PROC*)SDL_GL_GetProcAddress("glTextureStorage3D");
    glpfTextureStorage3DMultisample = (PFNGLTEXTURESTORAGE3DMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glTextureStorage3DMultisample");
    glpfTextureSubImage1D = (PFNGLTEXTURESUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glTextureSubImage1D");
    glpfTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glTextureSubImage2D");
    glpfTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glTextureSubImage3D");
    glpfTransformFeedbackBufferBase = (PFNGLTRANSFORMFEEDBACKBUFFERBASE_PROC*)SDL_GL_GetProcAddress("glTransformFeedbackBufferBase");
    glpfTransformFeedbackBufferRange = (PFNGLTRANSFORMFEEDBACKBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glTransformFeedbackBufferRange");
    glpfUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFER_PROC*)SDL_GL_GetProcAddress("glUnmapNamedBuffer");
    glpfVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDING_PROC*)SDL_GL_GetProcAddress("glVertexArrayAttribBinding");
    glpfVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMAT_PROC*)SDL_GL_GetProcAddress("glVertexArrayAttribFormat");
    glpfVertexArrayAttribIFormat = (PFNGLVERTEXARRAYATTRIBIFORMAT_PROC*)SDL_GL_GetProcAddress("glVertexArrayAttribIFormat");
    glpfVertexArrayAttribLFormat = (PFNGLVERTEXARRAYATTRIBLFORMAT_PROC*)SDL_GL_GetProcAddress("glVertexArrayAttribLFormat");
    glpfVertexArrayBindingDivisor = (PFNGLVERTEXARRAYBINDINGDIVISOR_PROC*)SDL_GL_GetProcAddress("glVertexArrayBindingDivisor");
    glpfVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFER_PROC*)SDL_GL_GetProcAddress("glVertexArrayElementBuffer");
    glpfVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFER_PROC*)SDL_GL_GetProcAddress("glVertexArrayVertexBuffer");
    glpfVertexArrayVertexBuffers = (PFNGLVERTEXARRAYVERTEXBUFFERS_PROC*)SDL_GL_GetProcAddress("glVertexArrayVertexBuffers");

    /* GL_ARB_draw_indirect */

    glpfDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECT_PROC*)SDL_GL_GetProcAddress("glDrawArraysIndirect");
    glpfDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECT_PROC*)SDL_GL_GetProcAddress("glDrawElementsIndirect");

    /* GL_ARB_instanced_arrays */

    glpfVertexAttribDivisorARB = (PFNGLVERTEXATTRIBDIVISORARB_PROC*)SDL_GL_GetProcAddress("glVertexAttribDivisorARB");

    /* GL_ARB_multi_draw_indirect */

    glpfMultiDrawArraysIndirect = (PFNGLMULTIDRAWARRAYSINDIRECT_PROC*)SDL_GL_GetProcAddress("glMultiDrawArraysIndirect");
    glpfMultiDrawElementsIndirect = (PFNGLMULTIDRAWELEMENTSINDIRECT_PROC*)SDL_GL_GetProcAddress("glMultiDrawElementsIndirect");

    /* GL_ARB_shader_image_load_store */

    glpfBindImageTexture = (PFNGLBINDIMAGETEXTURE_PROC*)SDL_GL_GetProcAddress("glBindImageTexture");
    glpfMemoryBarrier = (PFNGLMEMORYBARRIER_PROC*)SDL_GL_GetProcAddress("glMemoryBarrier");

    /* GL_ARB_shader_storage_buffer_object */

    glpfShaderStorageBlockBinding = (PFNGLSHADERSTORAGEBLOCKBINDING_PROC*)SDL_GL_GetProcAddress("glShaderStorageBlockBinding");

    /* GL_ARB_transform_feedback2 */

    glpfBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glBindTransformFeedback");
    glpfDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKS_PROC*)SDL_GL_GetProcAddress("glDeleteTransformFeedbacks");
    glpfDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glDrawTransformFeedback");
    glpfGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKS_PROC*)SDL_GL_GetProcAddress("glGenTransformFeedbacks");
    glpfIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glIsTransformFeedback");
    glpfPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glPauseTransformFeedback");
    glpfResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glResumeTransformFeedback");

    /* GL_VERSION_1_0 */

    glpfBlendFunc = (PFNGLBLENDFUNC_PROC*)SDL_GL_GetProcAddress("glBlendFunc");
    glpfClear = (PFNGLCLEAR_PROC*)SDL_GL_GetProcAddress("glClear");
    glpfClearColor = (PFNGLCLEARCOLOR_PROC*)SDL_GL_GetProcAddress("glClearColor");
    glpfClearDepth = (PFNGLCLEARDEPTH_PROC*)SDL_GL_GetProcAddress("glClearDepth");
    glpfClearStencil = (PFNGLCLEARSTENCIL_PROC*)SDL_GL_GetProcAddress("glClearStencil");
    glpfColorMask = (PFNGLCOLORMASK_PROC*)SDL_GL_GetProcAddress("glColorMask");
    glpfCullFace = (PFNGLCULLFACE_PROC*)SDL_GL_GetProcAddress("glCullFace");
    glpfDepthFunc = (PFNGLDEPTHFUNC_PROC*)SDL_GL_GetProcAddress("glDepthFunc");
    glpfDepthMask = (PFNGLDEPTHMASK_PROC*)SDL_GL_GetProcAddress("glDepthMask");
    glpfDepthRange = (PFNGLDEPTHRANGE_PROC*)SDL_GL_GetProcAddress("glDepthRange");
    glpfDisable = (PFNGLDISABLE_PROC*)SDL_GL_GetProcAddress("glDisable");
    glpfDrawBuffer = (PFNGLDRAWBUFFER_PROC*)SDL_GL_GetProcAddress("glDrawBuffer");
    glpfEnable = (PFNGLENABLE_PROC*)SDL_GL_GetProcAddress("glEnable");
    glpfFinish = (PFNGLFINISH_PROC*)SDL_GL_GetProcAddress("glFinish");
    glpfFlush = (PFNGLFLUSH_PROC*)SDL_GL_GetProcAddress("glFlush");
    glpfFrontFace = (PFNGLFRONTFACE_PROC*)SDL_GL_GetProcAddress("glFrontFace");
    glpfGetBooleanv = (PFNGLGETBOOLEANV_PROC*)SDL_GL_GetProcAddress("glGetBooleanv");
    glpfGetDoublev = (PFNGLGETDOUBLEV_PROC*)SDL_GL_GetProcAddress("glGetDoublev");
    glpfGetError = (PFNGLGETERROR_PROC*)SDL_GL_GetProcAddress("glGetError");
    glpfGetFloatv = (PFNGLGETFLOATV_PROC*)SDL_GL_GetProcAddress("glGetFloatv");
    glpfGetIntegerv = (PFNGLGETINTEGERV_PROC*)SDL_GL_GetProcAddress("glGetIntegerv");
    glpfGetString = (PFNGLGETSTRING_PROC*)SDL_GL_GetProcAddress("glGetString");
    glpfGetTexImage = (PFNGLGETTEXIMAGE_PROC*)SDL_GL_GetProcAddress("glGetTexImage");
    glpfGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glGetTexLevelParameterfv");
    glpfGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetTexLevelParameteriv");
    glpfGetTexParameterfv = (PFNGLGETTEXPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glGetTexParameterfv");
    glpfGetTexParameteriv = (PFNGLGETTEXPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetTexParameteriv");
    glpfHint = (PFNGLHINT_PROC*)SDL_GL_GetProcAddress("glHint");
    glpfIsEnabled = (PFNGLISENABLED_PROC*)SDL_GL_GetProcAddress("glIsEnabled");
    glpfLineWidth = (PFNGLLINEWIDTH_PROC*)SDL_GL_GetProcAddress("glLineWidth");
    glpfLogicOp = (PFNGLLOGICOP_PROC*)SDL_GL_GetProcAddress("glLogicOp");
    glpfPixelStoref = (PFNGLPIXELSTOREF_PROC*)SDL_GL_GetProcAddress("glPixelStoref");
    glpfPixelStorei = (PFNGLPIXELSTOREI_PROC*)SDL_GL_GetProcAddress("glPixelStorei");
    glpfPointSize = (PFNGLPOINTSIZE_PROC*)SDL_GL_GetProcAddress("glPointSize");
    glpfPolygonMode = (PFNGLPOLYGONMODE_PROC*)SDL_GL_GetProcAddress("glPolygonMode");
    glpfReadBuffer = (PFNGLREADBUFFER_PROC*)SDL_GL_GetProcAddress("glReadBuffer");
    glpfReadPixels = (PFNGLREADPIXELS_PROC*)SDL_GL_GetProcAddress("glReadPixels");
    glpfScissor = (PFNGLSCISSOR_PROC*)SDL_GL_GetProcAddress("glScissor");
    glpfStencilFunc = (PFNGLSTENCILFUNC_PROC*)SDL_GL_GetProcAddress("glStencilFunc");
    glpfStencilMask = (PFNGLSTENCILMASK_PROC*)SDL_GL_GetProcAddress("glStencilMask");
    glpfStencilOp = (PFNGLSTENCILOP_PROC*)SDL_GL_GetProcAddress("glStencilOp");
    glpfTexImage1D = (PFNGLTEXIMAGE1D_PROC*)SDL_GL_GetProcAddress("glTexImage1D");
    glpfTexImage2D = (PFNGLTEXIMAGE2D_PROC*)SDL_GL_GetProcAddress("glTexImage2D");
    glpfTexParameterf = (PFNGLTEXPARAMETERF_PROC*)SDL_GL_GetProcAddress("glTexParameterf");
    glpfTexParameterfv = (PFNGLTEXPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glTexParameterfv");
    glpfTexParameteri = (PFNGLTEXPARAMETERI_PROC*)SDL_GL_GetProcAddress("glTexParameteri");
    glpfTexParameteriv = (PFNGLTEXPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glTexParameteriv");
    glpfViewport = (PFNGLVIEWPORT_PROC*)SDL_GL_GetProcAddress("glViewport");

    /* GL_VERSION_1_1 */

    glpfBindTexture = (PFNGLBINDTEXTURE_PROC*)SDL_GL_GetProcAddress("glBindTexture");
    glpfCopyTexImage1D = (PFNGLCOPYTEXIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCopyTexImage1D");
    glpfCopyTexImage2D = (PFNGLCOPYTEXIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCopyTexImage2D");
    glpfCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCopyTexSubImage1D");
    glpfCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCopyTexSubImage2D");
    glpfDeleteTextures = (PFNGLDELETETEXTURES_PROC*)SDL_GL_GetProcAddress("glDeleteTextures");
    glpfDrawArrays = (PFNGLDRAWARRAYS_PROC*)SDL_GL_GetProcAddress("glDrawArrays");
    glpfDrawElements = (PFNGLDRAWELEMENTS_PROC*)SDL_GL_GetProcAddress("glDrawElements");
    glpfGenTextures = (PFNGLGENTEXTURES_PROC*)SDL_GL_GetProcAddress("glGenTextures");
    glpfIsTexture = (PFNGLISTEXTURE_PROC*)SDL_GL_GetProcAddress("glIsTexture");
    glpfPolygonOffset = (PFNGLPOLYGONOFFSET_PROC*)SDL_GL_GetProcAddress("glPolygonOffset");
    glpfTexSubImage1D = (PFNGLTEXSUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glTexSubImage1D");
    glpfTexSubImage2D = (PFNGLTEXSUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glTexSubImage2D");

    /* GL_VERSION_1_2 */

    glpfCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glCopyTexSubImage3D");
    glpfDrawRangeElements = (PFNGLDRAWRANGEELEMENTS_PROC*)SDL_GL_GetProcAddress("glDrawRangeElements");
    glpfTexImage3D = (PFNGLTEXIMAGE3D_PROC*)SDL_GL_GetProcAddress("glTexImage3D");
    glpfTexSubImage3D = (PFNGLTEXSUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glTexSubImage3D");

    /* GL_VERSION_1_3 */

    glpfActiveTexture = (PFNGLACTIVETEXTURE_PROC*)SDL_GL_GetProcAddress("glActiveTexture");
    glpfCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCompressedTexImage1D");
    glpfCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCompressedTexImage2D");
    glpfCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3D_PROC*)SDL_GL_GetProcAddress("glCompressedTexImage3D");
    glpfCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1D_PROC*)SDL_GL_GetProcAddress("glCompressedTexSubImage1D");
    glpfCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2D_PROC*)SDL_GL_GetProcAddress("glCompressedTexSubImage2D");
    glpfCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3D_PROC*)SDL_GL_GetProcAddress("glCompressedTexSubImage3D");
    glpfGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGE_PROC*)SDL_GL_GetProcAddress("glGetCompressedTexImage");
    glpfSampleCoverage = (PFNGLSAMPLECOVERAGE_PROC*)SDL_GL_GetProcAddress("glSampleCoverage");

    /* GL_VERSION_1_4 */

    glpfBlendColor = (PFNGLBLENDCOLOR_PROC*)SDL_GL_GetProcAddress("glBlendColor");
    glpfBlendEquation = (PFNGLBLENDEQUATION_PROC*)SDL_GL_GetProcAddress("glBlendEquation");
    glpfBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATE_PROC*)SDL_GL_GetProcAddress("glBlendFuncSeparate");
    glpfMultiDrawArrays = (PFNGLMULTIDRAWARRAYS_PROC*)SDL_GL_GetProcAddress("glMultiDrawArrays");
    glpfMultiDrawElements = (PFNGLMULTIDRAWELEMENTS_PROC*)SDL_GL_GetProcAddress("glMultiDrawElements");
    glpfPointParameterf = (PFNGLPOINTPARAMETERF_PROC*)SDL_GL_GetProcAddress("glPointParameterf");
    glpfPointParameterfv = (PFNGLPOINTPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glPointParameterfv");
    glpfPointParameteri = (PFNGLPOINTPARAMETERI_PROC*)SDL_GL_GetProcAddress("glPointParameteri");
    glpfPointParameteriv = (PFNGLPOINTPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glPointParameteriv");

    /* GL_VERSION_1_5 */

    glpfBeginQuery = (PFNGLBEGINQUERY_PROC*)SDL_GL_GetProcAddress("glBeginQuery");
    glpfBindBuffer = (PFNGLBINDBUFFER_PROC*)SDL_GL_GetProcAddress("glBindBuffer");
    glpfBufferData = (PFNGLBUFFERDATA_PROC*)SDL_GL_GetProcAddress("glBufferData");
    glpfBufferSubData = (PFNGLBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glBufferSubData");
    glpfDeleteBuffers = (PFNGLDELETEBUFFERS_PROC*)SDL_GL_GetProcAddress("glDeleteBuffers");
    glpfDeleteQueries = (PFNGLDELETEQUERIES_PROC*)SDL_GL_GetProcAddress("glDeleteQueries");
    glpfEndQuery = (PFNGLENDQUERY_PROC*)SDL_GL_GetProcAddress("glEndQuery");
    glpfGenBuffers = (PFNGLGENBUFFERS_PROC*)SDL_GL_GetProcAddress("glGenBuffers");
    glpfGenQueries = (PFNGLGENQUERIES_PROC*)SDL_GL_GetProcAddress("glGenQueries");
    glpfGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetBufferParameteriv");
    glpfGetBufferPointerv = (PFNGLGETBUFFERPOINTERV_PROC*)SDL_GL_GetProcAddress("glGetBufferPointerv");
    glpfGetBufferSubData = (PFNGLGETBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glGetBufferSubData");
    glpfGetQueryObjectiv = (PFNGLGETQUERYOBJECTIV_PROC*)SDL_GL_GetProcAddress("glGetQueryObjectiv");
    glpfGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIV_PROC*)SDL_GL_GetProcAddress("glGetQueryObjectuiv");
    glpfGetQueryiv = (PFNGLGETQUERYIV_PROC*)SDL_GL_GetProcAddress("glGetQueryiv");
    glpfIsBuffer = (PFNGLISBUFFER_PROC*)SDL_GL_GetProcAddress("glIsBuffer");
    glpfIsQuery = (PFNGLISQUERY_PROC*)SDL_GL_GetProcAddress("glIsQuery");
    glpfMapBuffer = (PFNGLMAPBUFFER_PROC*)SDL_GL_GetProcAddress("glMapBuffer");
    glpfUnmapBuffer = (PFNGLUNMAPBUFFER_PROC*)SDL_GL_GetProcAddress("glUnmapBuffer");

    /* GL_VERSION_2_0 */

    glpfAttachShader = (PFNGLATTACHSHADER_PROC*)SDL_GL_GetProcAddress("glAttachShader");
    glpfBindAttribLocation = (PFNGLBINDATTRIBLOCATION_PROC*)SDL_GL_GetProcAddress("glBindAttribLocation");
    glpfBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATE_PROC*)SDL_GL_GetProcAddress("glBlendEquationSeparate");
    glpfCompileShader = (PFNGLCOMPILESHADER_PROC*)SDL_GL_GetProcAddress("glCompileShader");
    glpfCreateProgram = (PFNGLCREATEPROGRAM_PROC*)SDL_GL_GetProcAddress("glCreateProgram");
    glpfCreateShader = (PFNGLCREATESHADER_PROC*)SDL_GL_GetProcAddress("glCreateShader");
    glpfDeleteProgram = (PFNGLDELETEPROGRAM_PROC*)SDL_GL_GetProcAddress("glDeleteProgram");
    glpfDeleteShader = (PFNGLDELETESHADER_PROC*)SDL_GL_GetProcAddress("glDeleteShader");
    glpfDetachShader = (PFNGLDETACHSHADER_PROC*)SDL_GL_GetProcAddress("glDetachShader");
    glpfDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAY_PROC*)SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    glpfDrawBuffers = (PFNGLDRAWBUFFERS_PROC*)SDL_GL_GetProcAddress("glDrawBuffers");
    glpfEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAY_PROC*)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glpfGetActiveAttrib = (PFNGLGETACTIVEATTRIB_PROC*)SDL_GL_GetProcAddress("glGetActiveAttrib");
    glpfGetActiveUniform = (PFNGLGETACTIVEUNIFORM_PROC*)SDL_GL_GetProcAddress("glGetActiveUniform");
    glpfGetAttachedShaders = (PFNGLGETATTACHEDSHADERS_PROC*)SDL_GL_GetProcAddress("glGetAttachedShaders");
    glpfGetAttribLocation = (PFNGLGETATTRIBLOCATION_PROC*)SDL_GL_GetProcAddress("glGetAttribLocation");
    glpfGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOG_PROC*)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glpfGetProgramiv = (PFNGLGETPROGRAMIV_PROC*)SDL_GL_GetProcAddress("glGetProgramiv");
    glpfGetShaderInfoLog = (PFNGLGETSHADERINFOLOG_PROC*)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glpfGetShaderSource = (PFNGLGETSHADERSOURCE_PROC*)SDL_GL_GetProcAddress("glGetShaderSource");
    glpfGetShaderiv = (PFNGLGETSHADERIV_PROC*)SDL_GL_GetProcAddress("glGetShaderiv");
    glpfGetUniformLocation = (PFNGLGETUNIFORMLOCATION_PROC*)SDL_GL_GetProcAddress("glGetUniformLocation");
    glpfGetUniformfv = (PFNGLGETUNIFORMFV_PROC*)SDL_GL_GetProcAddress("glGetUniformfv");
    glpfGetUniformiv = (PFNGLGETUNIFORMIV_PROC*)SDL_GL_GetProcAddress("glGetUniformiv");
    glpfGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribPointerv");
    glpfGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribdv");
    glpfGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribfv");
    glpfGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribiv");
    glpfIsProgram = (PFNGLISPROGRAM_PROC*)SDL_GL_GetProcAddress("glIsProgram");
    glpfIsShader = (PFNGLISSHADER_PROC*)SDL_GL_GetProcAddress("glIsShader");
    glpfLinkProgram = (PFNGLLINKPROGRAM_PROC*)SDL_GL_GetProcAddress("glLinkProgram");
    glpfShaderSource = (PFNGLSHADERSOURCE_PROC*)SDL_GL_GetProcAddress("glShaderSource");
    glpfStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATE_PROC*)SDL_GL_GetProcAddress("glStencilFuncSeparate");
    glpfStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATE_PROC*)SDL_GL_GetProcAddress("glStencilMaskSeparate");
    glpfStencilOpSeparate = (PFNGLSTENCILOPSEPARATE_PROC*)SDL_GL_GetProcAddress("glStencilOpSeparate");
    glpfUniform1f = (PFNGLUNIFORM1F_PROC*)SDL_GL_GetProcAddress("glUniform1f");
    glpfUniform1fv = (PFNGLUNIFORM1FV_PROC*)SDL_GL_GetProcAddress("glUniform1fv");
    glpfUniform1i = (PFNGLUNIFORM1I_PROC*)SDL_GL_GetProcAddress("glUniform1i");
    glpfUniform1iv = (PFNGLUNIFORM1IV_PROC*)SDL_GL_GetProcAddress("glUniform1iv");
    glpfUniform2f = (PFNGLUNIFORM2F_PROC*)SDL_GL_GetProcAddress("glUniform2f");
    glpfUniform2fv = (PFNGLUNIFORM2FV_PROC*)SDL_GL_GetProcAddress("glUniform2fv");
    glpfUniform2i = (PFNGLUNIFORM2I_PROC*)SDL_GL_GetProcAddress("glUniform2i");
    glpfUniform2iv = (PFNGLUNIFORM2IV_PROC*)SDL_GL_GetProcAddress("glUniform2iv");
    glpfUniform3f = (PFNGLUNIFORM3F_PROC*)SDL_GL_GetProcAddress("glUniform3f");
    glpfUniform3fv = (PFNGLUNIFORM3FV_PROC*)SDL_GL_GetProcAddress("glUniform3fv");
    glpfUniform3i = (PFNGLUNIFORM3I_PROC*)SDL_GL_GetProcAddress("glUniform3i");
    glpfUniform3iv = (PFNGLUNIFORM3IV_PROC*)SDL_GL_GetProcAddress("glUniform3iv");
    glpfUniform4f = (PFNGLUNIFORM4F_PROC*)SDL_GL_GetProcAddress("glUniform4f");
    glpfUniform4fv = (PFNGLUNIFORM4FV_PROC*)SDL_GL_GetProcAddress("glUniform4fv");
    glpfUniform4i = (PFNGLUNIFORM4I_PROC*)SDL_GL_GetProcAddress("glUniform4i");
    glpfUniform4iv = (PFNGLUNIFORM4IV_PROC*)SDL_GL_GetProcAddress("glUniform4iv");
    glpfUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix2fv");
    glpfUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix3fv");
    glpfUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    glpfUseProgram = (PFNGLUSEPROGRAM_PROC*)SDL_GL_GetProcAddress("glUseProgram");
    glpfValidateProgram = (PFNGLVALIDATEPROGRAM_PROC*)SDL_GL_GetProcAddress("glValidateProgram");
    glpfVertexAttrib1d = (PFNGLVERTEXATTRIB1D_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1d");
    glpfVertexAttrib1dv = (PFNGLVERTEXATTRIB1DV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1dv");
    glpfVertexAttrib1f = (PFNGLVERTEXATTRIB1F_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1f");
    glpfVertexAttrib1fv = (PFNGLVERTEXATTRIB1FV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1fv");
    glpfVertexAttrib1s = (PFNGLVERTEXATTRIB1S_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1s");
    glpfVertexAttrib1sv = (PFNGLVERTEXATTRIB1SV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib1sv");
    glpfVertexAttrib2d = (PFNGLVERTEXATTRIB2D_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2d");
    glpfVertexAttrib2dv = (PFNGLVERTEXATTRIB2DV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2dv");
    glpfVertexAttrib2f = (PFNGLVERTEXATTRIB2F_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2f");
    glpfVertexAttrib2fv = (PFNGLVERTEXATTRIB2FV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2fv");
    glpfVertexAttrib2s = (PFNGLVERTEXATTRIB2S_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2s");
    glpfVertexAttrib2sv = (PFNGLVERTEXATTRIB2SV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib2sv");
    glpfVertexAttrib3d = (PFNGLVERTEXATTRIB3D_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3d");
    glpfVertexAttrib3dv = (PFNGLVERTEXATTRIB3DV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3dv");
    glpfVertexAttrib3f = (PFNGLVERTEXATTRIB3F_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3f");
    glpfVertexAttrib3fv = (PFNGLVERTEXATTRIB3FV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3fv");
    glpfVertexAttrib3s = (PFNGLVERTEXATTRIB3S_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3s");
    glpfVertexAttrib3sv = (PFNGLVERTEXATTRIB3SV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib3sv");
    glpfVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nbv");
    glpfVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Niv");
    glpfVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nsv");
    glpfVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUB_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nub");
    glpfVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nubv");
    glpfVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nuiv");
    glpfVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4Nusv");
    glpfVertexAttrib4bv = (PFNGLVERTEXATTRIB4BV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4bv");
    glpfVertexAttrib4d = (PFNGLVERTEXATTRIB4D_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4d");
    glpfVertexAttrib4dv = (PFNGLVERTEXATTRIB4DV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4dv");
    glpfVertexAttrib4f = (PFNGLVERTEXATTRIB4F_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4f");
    glpfVertexAttrib4fv = (PFNGLVERTEXATTRIB4FV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4fv");
    glpfVertexAttrib4iv = (PFNGLVERTEXATTRIB4IV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4iv");
    glpfVertexAttrib4s = (PFNGLVERTEXATTRIB4S_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4s");
    glpfVertexAttrib4sv = (PFNGLVERTEXATTRIB4SV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4sv");
    glpfVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4ubv");
    glpfVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4uiv");
    glpfVertexAttrib4usv = (PFNGLVERTEXATTRIB4USV_PROC*)SDL_GL_GetProcAddress("glVertexAttrib4usv");
    glpfVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTER_PROC*)SDL_GL_GetProcAddress("glVertexAttribPointer");

    /* GL_VERSION_2_1 */

    glpfUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix2x3fv");
    glpfUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix2x4fv");
    glpfUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix3x2fv");
    glpfUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix3x4fv");
    glpfUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix4x2fv");
    glpfUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FV_PROC*)SDL_GL_GetProcAddress("glUniformMatrix4x3fv");

    /* GL_VERSION_3_0 */

    glpfBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDER_PROC*)SDL_GL_GetProcAddress("glBeginConditionalRender");
    glpfBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glBeginTransformFeedback");
    glpfBindBufferBase = (PFNGLBINDBUFFERBASE_PROC*)SDL_GL_GetProcAddress("glBindBufferBase");
    glpfBindBufferRange = (PFNGLBINDBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glBindBufferRange");
    glpfBindFragDataLocation = (PFNGLBINDFRAGDATALOCATION_PROC*)SDL_GL_GetProcAddress("glBindFragDataLocation");
    glpfBindFramebuffer = (PFNGLBINDFRAMEBUFFER_PROC*)SDL_GL_GetProcAddress("glBindFramebuffer");
    glpfBindRenderbuffer = (PFNGLBINDRENDERBUFFER_PROC*)SDL_GL_GetProcAddress("glBindRenderbuffer");
    glpfBindVertexArray = (PFNGLBINDVERTEXARRAY_PROC*)SDL_GL_GetProcAddress("glBindVertexArray");
    glpfBlitFramebuffer = (PFNGLBLITFRAMEBUFFER_PROC*)SDL_GL_GetProcAddress("glBlitFramebuffer");
    glpfCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUS_PROC*)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
    glpfClampColor = (PFNGLCLAMPCOLOR_PROC*)SDL_GL_GetProcAddress("glClampColor");
    glpfClearBufferfi = (PFNGLCLEARBUFFERFI_PROC*)SDL_GL_GetProcAddress("glClearBufferfi");
    glpfClearBufferfv = (PFNGLCLEARBUFFERFV_PROC*)SDL_GL_GetProcAddress("glClearBufferfv");
    glpfClearBufferiv = (PFNGLCLEARBUFFERIV_PROC*)SDL_GL_GetProcAddress("glClearBufferiv");
    glpfClearBufferuiv = (PFNGLCLEARBUFFERUIV_PROC*)SDL_GL_GetProcAddress("glClearBufferuiv");
    glpfColorMaski = (PFNGLCOLORMASKI_PROC*)SDL_GL_GetProcAddress("glColorMaski");
    glpfDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERS_PROC*)SDL_GL_GetProcAddress("glDeleteFramebuffers");
    glpfDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERS_PROC*)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
    glpfDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYS_PROC*)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glpfDisablei = (PFNGLDISABLEI_PROC*)SDL_GL_GetProcAddress("glDisablei");
    glpfEnablei = (PFNGLENABLEI_PROC*)SDL_GL_GetProcAddress("glEnablei");
    glpfEndConditionalRender = (PFNGLENDCONDITIONALRENDER_PROC*)SDL_GL_GetProcAddress("glEndConditionalRender");
    glpfEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACK_PROC*)SDL_GL_GetProcAddress("glEndTransformFeedback");
    glpfFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glFlushMappedBufferRange");
    glpfFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFER_PROC*)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
    glpfFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1D_PROC*)SDL_GL_GetProcAddress("glFramebufferTexture1D");
    glpfFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2D_PROC*)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    glpfFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3D_PROC*)SDL_GL_GetProcAddress("glFramebufferTexture3D");
    glpfFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYER_PROC*)SDL_GL_GetProcAddress("glFramebufferTextureLayer");
    glpfGenFramebuffers = (PFNGLGENFRAMEBUFFERS_PROC*)SDL_GL_GetProcAddress("glGenFramebuffers");
    glpfGenRenderbuffers = (PFNGLGENRENDERBUFFERS_PROC*)SDL_GL_GetProcAddress("glGenRenderbuffers");
    glpfGenVertexArrays = (PFNGLGENVERTEXARRAYS_PROC*)SDL_GL_GetProcAddress("glGenVertexArrays");
    glpfGenerateMipmap = (PFNGLGENERATEMIPMAP_PROC*)SDL_GL_GetProcAddress("glGenerateMipmap");
    glpfGetBooleani_v = (PFNGLGETBOOLEANI_V_PROC*)SDL_GL_GetProcAddress("glGetBooleani_v");
    glpfGetFragDataLocation = (PFNGLGETFRAGDATALOCATION_PROC*)SDL_GL_GetProcAddress("glGetFragDataLocation");
    glpfGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameteriv");
    glpfGetIntegeri_v = (PFNGLGETINTEGERI_V_PROC*)SDL_GL_GetProcAddress("glGetIntegeri_v");
    glpfGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetRenderbufferParameteriv");
    glpfGetStringi = (PFNGLGETSTRINGI_PROC*)SDL_GL_GetProcAddress("glGetStringi");
    glpfGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glGetTexParameterIiv");
    glpfGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glGetTexParameterIuiv");
    glpfGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYING_PROC*)SDL_GL_GetProcAddress("glGetTransformFeedbackVarying");
    glpfGetUniformuiv = (PFNGLGETUNIFORMUIV_PROC*)SDL_GL_GetProcAddress("glGetUniformuiv");
    glpfGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribIiv");
    glpfGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIV_PROC*)SDL_GL_GetProcAddress("glGetVertexAttribIuiv");
    glpfIsEnabledi = (PFNGLISENABLEDI_PROC*)SDL_GL_GetProcAddress("glIsEnabledi");
    glpfIsFramebuffer = (PFNGLISFRAMEBUFFER_PROC*)SDL_GL_GetProcAddress("glIsFramebuffer");
    glpfIsRenderbuffer = (PFNGLISRENDERBUFFER_PROC*)SDL_GL_GetProcAddress("glIsRenderbuffer");
    glpfIsVertexArray = (PFNGLISVERTEXARRAY_PROC*)SDL_GL_GetProcAddress("glIsVertexArray");
    glpfMapBufferRange = (PFNGLMAPBUFFERRANGE_PROC*)SDL_GL_GetProcAddress("glMapBufferRange");
    glpfRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGE_PROC*)SDL_GL_GetProcAddress("glRenderbufferStorage");
    glpfRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glRenderbufferStorageMultisample");
    glpfTexParameterIiv = (PFNGLTEXPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glTexParameterIiv");
    glpfTexParameterIuiv = (PFNGLTEXPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glTexParameterIuiv");
    glpfTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGS_PROC*)SDL_GL_GetProcAddress("glTransformFeedbackVaryings");
    glpfUniform1ui = (PFNGLUNIFORM1UI_PROC*)SDL_GL_GetProcAddress("glUniform1ui");
    glpfUniform1uiv = (PFNGLUNIFORM1UIV_PROC*)SDL_GL_GetProcAddress("glUniform1uiv");
    glpfUniform2ui = (PFNGLUNIFORM2UI_PROC*)SDL_GL_GetProcAddress("glUniform2ui");
    glpfUniform2uiv = (PFNGLUNIFORM2UIV_PROC*)SDL_GL_GetProcAddress("glUniform2uiv");
    glpfUniform3ui = (PFNGLUNIFORM3UI_PROC*)SDL_GL_GetProcAddress("glUniform3ui");
    glpfUniform3uiv = (PFNGLUNIFORM3UIV_PROC*)SDL_GL_GetProcAddress("glUniform3uiv");
    glpfUniform4ui = (PFNGLUNIFORM4UI_PROC*)SDL_GL_GetProcAddress("glUniform4ui");
    glpfUniform4uiv = (PFNGLUNIFORM4UIV_PROC*)SDL_GL_GetProcAddress("glUniform4uiv");
    glpfVertexAttribI1i = (PFNGLVERTEXATTRIBI1I_PROC*)SDL_GL_GetProcAddress("glVertexAttribI1i");
    glpfVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI1iv");
    glpfVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribI1ui");
    glpfVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI1uiv");
    glpfVertexAttribI2i = (PFNGLVERTEXATTRIBI2I_PROC*)SDL_GL_GetProcAddress("glVertexAttribI2i");
    glpfVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI2iv");
    glpfVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribI2ui");
    glpfVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI2uiv");
    glpfVertexAttribI3i = (PFNGLVERTEXATTRIBI3I_PROC*)SDL_GL_GetProcAddress("glVertexAttribI3i");
    glpfVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI3iv");
    glpfVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribI3ui");
    glpfVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI3uiv");
    glpfVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4bv");
    glpfVertexAttribI4i = (PFNGLVERTEXATTRIBI4I_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4i");
    glpfVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4iv");
    glpfVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4sv");
    glpfVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4ubv");
    glpfVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4ui");
    glpfVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4uiv");
    glpfVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USV_PROC*)SDL_GL_GetProcAddress("glVertexAttribI4usv");
    glpfVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTER_PROC*)SDL_GL_GetProcAddress("glVertexAttribIPointer");

    /* GL_VERSION_3_1 */

    glpfCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATA_PROC*)SDL_GL_GetProcAddress("glCopyBufferSubData");
    glpfDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCED_PROC*)SDL_GL_GetProcAddress("glDrawArraysInstanced");
    glpfDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCED_PROC*)SDL_GL_GetProcAddress("glDrawElementsInstanced");
    glpfGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAME_PROC*)SDL_GL_GetProcAddress("glGetActiveUniformBlockName");
    glpfGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIV_PROC*)SDL_GL_GetProcAddress("glGetActiveUniformBlockiv");
    glpfGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAME_PROC*)SDL_GL_GetProcAddress("glGetActiveUniformName");
    glpfGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIV_PROC*)SDL_GL_GetProcAddress("glGetActiveUniformsiv");
    glpfGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEX_PROC*)SDL_GL_GetProcAddress("glGetUniformBlockIndex");
    glpfGetUniformIndices = (PFNGLGETUNIFORMINDICES_PROC*)SDL_GL_GetProcAddress("glGetUniformIndices");
    glpfPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEX_PROC*)SDL_GL_GetProcAddress("glPrimitiveRestartIndex");
    glpfTexBuffer = (PFNGLTEXBUFFER_PROC*)SDL_GL_GetProcAddress("glTexBuffer");
    glpfUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDING_PROC*)SDL_GL_GetProcAddress("glUniformBlockBinding");

    /* GL_VERSION_3_2 */

    glpfClientWaitSync = (PFNGLCLIENTWAITSYNC_PROC*)SDL_GL_GetProcAddress("glClientWaitSync");
    glpfDeleteSync = (PFNGLDELETESYNC_PROC*)SDL_GL_GetProcAddress("glDeleteSync");
    glpfDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEX_PROC*)SDL_GL_GetProcAddress("glDrawElementsBaseVertex");
    glpfDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEX_PROC*)SDL_GL_GetProcAddress("glDrawElementsInstancedBaseVertex");
    glpfDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEX_PROC*)SDL_GL_GetProcAddress("glDrawRangeElementsBaseVertex");
    glpfFenceSync = (PFNGLFENCESYNC_PROC*)SDL_GL_GetProcAddress("glFenceSync");
    glpfFramebufferTexture = (PFNGLFRAMEBUFFERTEXTURE_PROC*)SDL_GL_GetProcAddress("glFramebufferTexture");
    glpfGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64V_PROC*)SDL_GL_GetProcAddress("glGetBufferParameteri64v");
    glpfGetInteger64i_v = (PFNGLGETINTEGER64I_V_PROC*)SDL_GL_GetProcAddress("glGetInteger64i_v");
    glpfGetInteger64v = (PFNGLGETINTEGER64V_PROC*)SDL_GL_GetProcAddress("glGetInteger64v");
    glpfGetMultisamplefv = (PFNGLGETMULTISAMPLEFV_PROC*)SDL_GL_GetProcAddress("glGetMultisamplefv");
    glpfGetSynciv = (PFNGLGETSYNCIV_PROC*)SDL_GL_GetProcAddress("glGetSynciv");
    glpfIsSync = (PFNGLISSYNC_PROC*)SDL_GL_GetProcAddress("glIsSync");
    glpfMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEX_PROC*)SDL_GL_GetProcAddress("glMultiDrawElementsBaseVertex");
    glpfProvokingVertex = (PFNGLPROVOKINGVERTEX_PROC*)SDL_GL_GetProcAddress("glProvokingVertex");
    glpfSampleMaski = (PFNGLSAMPLEMASKI_PROC*)SDL_GL_GetProcAddress("glSampleMaski");
    glpfTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glTexImage2DMultisample");
    glpfTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLE_PROC*)SDL_GL_GetProcAddress("glTexImage3DMultisample");
    glpfWaitSync = (PFNGLWAITSYNC_PROC*)SDL_GL_GetProcAddress("glWaitSync");

    /* GL_VERSION_3_3 */

    glpfBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXED_PROC*)SDL_GL_GetProcAddress("glBindFragDataLocationIndexed");
    glpfBindSampler = (PFNGLBINDSAMPLER_PROC*)SDL_GL_GetProcAddress("glBindSampler");
    glpfDeleteSamplers = (PFNGLDELETESAMPLERS_PROC*)SDL_GL_GetProcAddress("glDeleteSamplers");
    glpfGenSamplers = (PFNGLGENSAMPLERS_PROC*)SDL_GL_GetProcAddress("glGenSamplers");
    glpfGetFragDataIndex = (PFNGLGETFRAGDATAINDEX_PROC*)SDL_GL_GetProcAddress("glGetFragDataIndex");
    glpfGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64V_PROC*)SDL_GL_GetProcAddress("glGetQueryObjecti64v");
    glpfGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64V_PROC*)SDL_GL_GetProcAddress("glGetQueryObjectui64v");
    glpfGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glGetSamplerParameterIiv");
    glpfGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glGetSamplerParameterIuiv");
    glpfGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glGetSamplerParameterfv");
    glpfGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glGetSamplerParameteriv");
    glpfIsSampler = (PFNGLISSAMPLER_PROC*)SDL_GL_GetProcAddress("glIsSampler");
    glpfQueryCounter = (PFNGLQUERYCOUNTER_PROC*)SDL_GL_GetProcAddress("glQueryCounter");
    glpfSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIV_PROC*)SDL_GL_GetProcAddress("glSamplerParameterIiv");
    glpfSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIV_PROC*)SDL_GL_GetProcAddress("glSamplerParameterIuiv");
    glpfSamplerParameterf = (PFNGLSAMPLERPARAMETERF_PROC*)SDL_GL_GetProcAddress("glSamplerParameterf");
    glpfSamplerParameterfv = (PFNGLSAMPLERPARAMETERFV_PROC*)SDL_GL_GetProcAddress("glSamplerParameterfv");
    glpfSamplerParameteri = (PFNGLSAMPLERPARAMETERI_PROC*)SDL_GL_GetProcAddress("glSamplerParameteri");
    glpfSamplerParameteriv = (PFNGLSAMPLERPARAMETERIV_PROC*)SDL_GL_GetProcAddress("glSamplerParameteriv");
    glpfVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOR_PROC*)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    glpfVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribP1ui");
    glpfVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribP1uiv");
    glpfVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribP2ui");
    glpfVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribP2uiv");
    glpfVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribP3ui");
    glpfVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribP3uiv");
    glpfVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UI_PROC*)SDL_GL_GetProcAddress("glVertexAttribP4ui");
    glpfVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIV_PROC*)SDL_GL_GetProcAddress("glVertexAttribP4uiv");

}

/* ----------------------- Extension flag definitions ---------------------- */
int FLEXT_ARB_debug_output = GL_FALSE;
int FLEXT_ARB_direct_state_access = GL_FALSE;
int FLEXT_ARB_buffer_storage = GL_FALSE;
int FLEXT_ARB_multi_draw_indirect = GL_FALSE;
int FLEXT_ARB_draw_indirect = GL_FALSE;
int FLEXT_ARB_instanced_arrays = GL_FALSE;
int FLEXT_ARB_compute_shader = GL_FALSE;
int FLEXT_ARB_shader_image_load_store = GL_FALSE;
int FLEXT_ARB_transform_feedback2 = GL_FALSE;
int FLEXT_ARB_shader_storage_buffer_object = GL_FALSE;

/* ---------------------- Function pointer definitions --------------------- */

/* GL_ARB_buffer_storage */

PFNGLBUFFERSTORAGE_PROC* glpfBufferStorage = NULL;

/* GL_ARB_compute_shader */

PFNGLDISPATCHCOMPUTE_PROC* glpfDispatchCompute = NULL;
PFNGLDISPATCHCOMPUTEINDIRECT_PROC* glpfDispatchComputeIndirect = NULL;

/* GL_ARB_debug_output */

PFNGLDEBUGMESSAGECALLBACKARB_PROC* glpfDebugMessageCallbackARB = NULL;
PFNGLDEBUGMESSAGECONTROLARB_PROC* glpfDebugMessageControlARB = NULL;
PFNGLDEBUGMESSAGEINSERTARB_PROC* glpfDebugMessageInsertARB = NULL;
PFNGLGETDEBUGMESSAGELOGARB_PROC* glpfGetDebugMessageLogARB = NULL;

/* GL_ARB_direct_state_access */

PFNGLBINDTEXTUREUNIT_PROC* glpfBindTextureUnit = NULL;
PFNGLBLITNAMEDFRAMEBUFFER_PROC* glpfBlitNamedFramebuffer = NULL;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUS_PROC* glpfCheckNamedFramebufferStatus = NULL;
PFNGLCLEARNAMEDBUFFERDATA_PROC* glpfClearNamedBufferData = NULL;
PFNGLCLEARNAMEDBUFFERSUBDATA_PROC* glpfClearNamedBufferSubData = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERFI_PROC* glpfClearNamedFramebufferfi = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERFV_PROC* glpfClearNamedFramebufferfv = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERIV_PROC* glpfClearNamedFramebufferiv = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERUIV_PROC* glpfClearNamedFramebufferuiv = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1D_PROC* glpfCompressedTextureSubImage1D = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2D_PROC* glpfCompressedTextureSubImage2D = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3D_PROC* glpfCompressedTextureSubImage3D = NULL;
PFNGLCOPYNAMEDBUFFERSUBDATA_PROC* glpfCopyNamedBufferSubData = NULL;
PFNGLCOPYTEXTURESUBIMAGE1D_PROC* glpfCopyTextureSubImage1D = NULL;
PFNGLCOPYTEXTURESUBIMAGE2D_PROC* glpfCopyTextureSubImage2D = NULL;
PFNGLCOPYTEXTURESUBIMAGE3D_PROC* glpfCopyTextureSubImage3D = NULL;
PFNGLCREATEBUFFERS_PROC* glpfCreateBuffers = NULL;
PFNGLCREATEFRAMEBUFFERS_PROC* glpfCreateFramebuffers = NULL;
PFNGLCREATEPROGRAMPIPELINES_PROC* glpfCreateProgramPipelines = NULL;
PFNGLCREATEQUERIES_PROC* glpfCreateQueries = NULL;
PFNGLCREATERENDERBUFFERS_PROC* glpfCreateRenderbuffers = NULL;
PFNGLCREATESAMPLERS_PROC* glpfCreateSamplers = NULL;
PFNGLCREATETEXTURES_PROC* glpfCreateTextures = NULL;
PFNGLCREATETRANSFORMFEEDBACKS_PROC* glpfCreateTransformFeedbacks = NULL;
PFNGLCREATEVERTEXARRAYS_PROC* glpfCreateVertexArrays = NULL;
PFNGLDISABLEVERTEXARRAYATTRIB_PROC* glpfDisableVertexArrayAttrib = NULL;
PFNGLENABLEVERTEXARRAYATTRIB_PROC* glpfEnableVertexArrayAttrib = NULL;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGE_PROC* glpfFlushMappedNamedBufferRange = NULL;
PFNGLGENERATETEXTUREMIPMAP_PROC* glpfGenerateTextureMipmap = NULL;
PFNGLGETCOMPRESSEDTEXTUREIMAGE_PROC* glpfGetCompressedTextureImage = NULL;
PFNGLGETNAMEDBUFFERPARAMETERI64V_PROC* glpfGetNamedBufferParameteri64v = NULL;
PFNGLGETNAMEDBUFFERPARAMETERIV_PROC* glpfGetNamedBufferParameteriv = NULL;
PFNGLGETNAMEDBUFFERPOINTERV_PROC* glpfGetNamedBufferPointerv = NULL;
PFNGLGETNAMEDBUFFERSUBDATA_PROC* glpfGetNamedBufferSubData = NULL;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV_PROC* glpfGetNamedFramebufferAttachmentParameteriv = NULL;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIV_PROC* glpfGetNamedFramebufferParameteriv = NULL;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIV_PROC* glpfGetNamedRenderbufferParameteriv = NULL;
PFNGLGETQUERYBUFFEROBJECTI64V_PROC* glpfGetQueryBufferObjecti64v = NULL;
PFNGLGETQUERYBUFFEROBJECTIV_PROC* glpfGetQueryBufferObjectiv = NULL;
PFNGLGETQUERYBUFFEROBJECTUI64V_PROC* glpfGetQueryBufferObjectui64v = NULL;
PFNGLGETQUERYBUFFEROBJECTUIV_PROC* glpfGetQueryBufferObjectuiv = NULL;
PFNGLGETTEXTUREIMAGE_PROC* glpfGetTextureImage = NULL;
PFNGLGETTEXTURELEVELPARAMETERFV_PROC* glpfGetTextureLevelParameterfv = NULL;
PFNGLGETTEXTURELEVELPARAMETERIV_PROC* glpfGetTextureLevelParameteriv = NULL;
PFNGLGETTEXTUREPARAMETERIIV_PROC* glpfGetTextureParameterIiv = NULL;
PFNGLGETTEXTUREPARAMETERIUIV_PROC* glpfGetTextureParameterIuiv = NULL;
PFNGLGETTEXTUREPARAMETERFV_PROC* glpfGetTextureParameterfv = NULL;
PFNGLGETTEXTUREPARAMETERIV_PROC* glpfGetTextureParameteriv = NULL;
PFNGLGETTRANSFORMFEEDBACKI64_V_PROC* glpfGetTransformFeedbacki64_v = NULL;
PFNGLGETTRANSFORMFEEDBACKI_V_PROC* glpfGetTransformFeedbacki_v = NULL;
PFNGLGETTRANSFORMFEEDBACKIV_PROC* glpfGetTransformFeedbackiv = NULL;
PFNGLGETVERTEXARRAYINDEXED64IV_PROC* glpfGetVertexArrayIndexed64iv = NULL;
PFNGLGETVERTEXARRAYINDEXEDIV_PROC* glpfGetVertexArrayIndexediv = NULL;
PFNGLGETVERTEXARRAYIV_PROC* glpfGetVertexArrayiv = NULL;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATA_PROC* glpfInvalidateNamedFramebufferData = NULL;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATA_PROC* glpfInvalidateNamedFramebufferSubData = NULL;
PFNGLMAPNAMEDBUFFER_PROC* glpfMapNamedBuffer = NULL;
PFNGLMAPNAMEDBUFFERRANGE_PROC* glpfMapNamedBufferRange = NULL;
PFNGLNAMEDBUFFERDATA_PROC* glpfNamedBufferData = NULL;
PFNGLNAMEDBUFFERSTORAGE_PROC* glpfNamedBufferStorage = NULL;
PFNGLNAMEDBUFFERSUBDATA_PROC* glpfNamedBufferSubData = NULL;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFER_PROC* glpfNamedFramebufferDrawBuffer = NULL;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERS_PROC* glpfNamedFramebufferDrawBuffers = NULL;
PFNGLNAMEDFRAMEBUFFERPARAMETERI_PROC* glpfNamedFramebufferParameteri = NULL;
PFNGLNAMEDFRAMEBUFFERREADBUFFER_PROC* glpfNamedFramebufferReadBuffer = NULL;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFER_PROC* glpfNamedFramebufferRenderbuffer = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE_PROC* glpfNamedFramebufferTexture = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYER_PROC* glpfNamedFramebufferTextureLayer = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGE_PROC* glpfNamedRenderbufferStorage = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLE_PROC* glpfNamedRenderbufferStorageMultisample = NULL;
PFNGLTEXTUREBUFFER_PROC* glpfTextureBuffer = NULL;
PFNGLTEXTUREBUFFERRANGE_PROC* glpfTextureBufferRange = NULL;
PFNGLTEXTUREPARAMETERIIV_PROC* glpfTextureParameterIiv = NULL;
PFNGLTEXTUREPARAMETERIUIV_PROC* glpfTextureParameterIuiv = NULL;
PFNGLTEXTUREPARAMETERF_PROC* glpfTextureParameterf = NULL;
PFNGLTEXTUREPARAMETERFV_PROC* glpfTextureParameterfv = NULL;
PFNGLTEXTUREPARAMETERI_PROC* glpfTextureParameteri = NULL;
PFNGLTEXTUREPARAMETERIV_PROC* glpfTextureParameteriv = NULL;
PFNGLTEXTURESTORAGE1D_PROC* glpfTextureStorage1D = NULL;
PFNGLTEXTURESTORAGE2D_PROC* glpfTextureStorage2D = NULL;
PFNGLTEXTURESTORAGE2DMULTISAMPLE_PROC* glpfTextureStorage2DMultisample = NULL;
PFNGLTEXTURESTORAGE3D_PROC* glpfTextureStorage3D = NULL;
PFNGLTEXTURESTORAGE3DMULTISAMPLE_PROC* glpfTextureStorage3DMultisample = NULL;
PFNGLTEXTURESUBIMAGE1D_PROC* glpfTextureSubImage1D = NULL;
PFNGLTEXTURESUBIMAGE2D_PROC* glpfTextureSubImage2D = NULL;
PFNGLTEXTURESUBIMAGE3D_PROC* glpfTextureSubImage3D = NULL;
PFNGLTRANSFORMFEEDBACKBUFFERBASE_PROC* glpfTransformFeedbackBufferBase = NULL;
PFNGLTRANSFORMFEEDBACKBUFFERRANGE_PROC* glpfTransformFeedbackBufferRange = NULL;
PFNGLUNMAPNAMEDBUFFER_PROC* glpfUnmapNamedBuffer = NULL;
PFNGLVERTEXARRAYATTRIBBINDING_PROC* glpfVertexArrayAttribBinding = NULL;
PFNGLVERTEXARRAYATTRIBFORMAT_PROC* glpfVertexArrayAttribFormat = NULL;
PFNGLVERTEXARRAYATTRIBIFORMAT_PROC* glpfVertexArrayAttribIFormat = NULL;
PFNGLVERTEXARRAYATTRIBLFORMAT_PROC* glpfVertexArrayAttribLFormat = NULL;
PFNGLVERTEXARRAYBINDINGDIVISOR_PROC* glpfVertexArrayBindingDivisor = NULL;
PFNGLVERTEXARRAYELEMENTBUFFER_PROC* glpfVertexArrayElementBuffer = NULL;
PFNGLVERTEXARRAYVERTEXBUFFER_PROC* glpfVertexArrayVertexBuffer = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERS_PROC* glpfVertexArrayVertexBuffers = NULL;

/* GL_ARB_draw_indirect */

PFNGLDRAWARRAYSINDIRECT_PROC* glpfDrawArraysIndirect = NULL;
PFNGLDRAWELEMENTSINDIRECT_PROC* glpfDrawElementsIndirect = NULL;

/* GL_ARB_instanced_arrays */

PFNGLVERTEXATTRIBDIVISORARB_PROC* glpfVertexAttribDivisorARB = NULL;

/* GL_ARB_multi_draw_indirect */

PFNGLMULTIDRAWARRAYSINDIRECT_PROC* glpfMultiDrawArraysIndirect = NULL;
PFNGLMULTIDRAWELEMENTSINDIRECT_PROC* glpfMultiDrawElementsIndirect = NULL;

/* GL_ARB_shader_image_load_store */

PFNGLBINDIMAGETEXTURE_PROC* glpfBindImageTexture = NULL;
PFNGLMEMORYBARRIER_PROC* glpfMemoryBarrier = NULL;

/* GL_ARB_shader_storage_buffer_object */

PFNGLSHADERSTORAGEBLOCKBINDING_PROC* glpfShaderStorageBlockBinding = NULL;

/* GL_ARB_transform_feedback2 */

PFNGLBINDTRANSFORMFEEDBACK_PROC* glpfBindTransformFeedback = NULL;
PFNGLDELETETRANSFORMFEEDBACKS_PROC* glpfDeleteTransformFeedbacks = NULL;
PFNGLDRAWTRANSFORMFEEDBACK_PROC* glpfDrawTransformFeedback = NULL;
PFNGLGENTRANSFORMFEEDBACKS_PROC* glpfGenTransformFeedbacks = NULL;
PFNGLISTRANSFORMFEEDBACK_PROC* glpfIsTransformFeedback = NULL;
PFNGLPAUSETRANSFORMFEEDBACK_PROC* glpfPauseTransformFeedback = NULL;
PFNGLRESUMETRANSFORMFEEDBACK_PROC* glpfResumeTransformFeedback = NULL;

/* GL_VERSION_1_0 */

PFNGLBLENDFUNC_PROC* glpfBlendFunc = NULL;
PFNGLCLEAR_PROC* glpfClear = NULL;
PFNGLCLEARCOLOR_PROC* glpfClearColor = NULL;
PFNGLCLEARDEPTH_PROC* glpfClearDepth = NULL;
PFNGLCLEARSTENCIL_PROC* glpfClearStencil = NULL;
PFNGLCOLORMASK_PROC* glpfColorMask = NULL;
PFNGLCULLFACE_PROC* glpfCullFace = NULL;
PFNGLDEPTHFUNC_PROC* glpfDepthFunc = NULL;
PFNGLDEPTHMASK_PROC* glpfDepthMask = NULL;
PFNGLDEPTHRANGE_PROC* glpfDepthRange = NULL;
PFNGLDISABLE_PROC* glpfDisable = NULL;
PFNGLDRAWBUFFER_PROC* glpfDrawBuffer = NULL;
PFNGLENABLE_PROC* glpfEnable = NULL;
PFNGLFINISH_PROC* glpfFinish = NULL;
PFNGLFLUSH_PROC* glpfFlush = NULL;
PFNGLFRONTFACE_PROC* glpfFrontFace = NULL;
PFNGLGETBOOLEANV_PROC* glpfGetBooleanv = NULL;
PFNGLGETDOUBLEV_PROC* glpfGetDoublev = NULL;
PFNGLGETERROR_PROC* glpfGetError = NULL;
PFNGLGETFLOATV_PROC* glpfGetFloatv = NULL;
PFNGLGETINTEGERV_PROC* glpfGetIntegerv = NULL;
PFNGLGETSTRING_PROC* glpfGetString = NULL;
PFNGLGETTEXIMAGE_PROC* glpfGetTexImage = NULL;
PFNGLGETTEXLEVELPARAMETERFV_PROC* glpfGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIV_PROC* glpfGetTexLevelParameteriv = NULL;
PFNGLGETTEXPARAMETERFV_PROC* glpfGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIV_PROC* glpfGetTexParameteriv = NULL;
PFNGLHINT_PROC* glpfHint = NULL;
PFNGLISENABLED_PROC* glpfIsEnabled = NULL;
PFNGLLINEWIDTH_PROC* glpfLineWidth = NULL;
PFNGLLOGICOP_PROC* glpfLogicOp = NULL;
PFNGLPIXELSTOREF_PROC* glpfPixelStoref = NULL;
PFNGLPIXELSTOREI_PROC* glpfPixelStorei = NULL;
PFNGLPOINTSIZE_PROC* glpfPointSize = NULL;
PFNGLPOLYGONMODE_PROC* glpfPolygonMode = NULL;
PFNGLREADBUFFER_PROC* glpfReadBuffer = NULL;
PFNGLREADPIXELS_PROC* glpfReadPixels = NULL;
PFNGLSCISSOR_PROC* glpfScissor = NULL;
PFNGLSTENCILFUNC_PROC* glpfStencilFunc = NULL;
PFNGLSTENCILMASK_PROC* glpfStencilMask = NULL;
PFNGLSTENCILOP_PROC* glpfStencilOp = NULL;
PFNGLTEXIMAGE1D_PROC* glpfTexImage1D = NULL;
PFNGLTEXIMAGE2D_PROC* glpfTexImage2D = NULL;
PFNGLTEXPARAMETERF_PROC* glpfTexParameterf = NULL;
PFNGLTEXPARAMETERFV_PROC* glpfTexParameterfv = NULL;
PFNGLTEXPARAMETERI_PROC* glpfTexParameteri = NULL;
PFNGLTEXPARAMETERIV_PROC* glpfTexParameteriv = NULL;
PFNGLVIEWPORT_PROC* glpfViewport = NULL;

/* GL_VERSION_1_1 */

PFNGLBINDTEXTURE_PROC* glpfBindTexture = NULL;
PFNGLCOPYTEXIMAGE1D_PROC* glpfCopyTexImage1D = NULL;
PFNGLCOPYTEXIMAGE2D_PROC* glpfCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE1D_PROC* glpfCopyTexSubImage1D = NULL;
PFNGLCOPYTEXSUBIMAGE2D_PROC* glpfCopyTexSubImage2D = NULL;
PFNGLDELETETEXTURES_PROC* glpfDeleteTextures = NULL;
PFNGLDRAWARRAYS_PROC* glpfDrawArrays = NULL;
PFNGLDRAWELEMENTS_PROC* glpfDrawElements = NULL;
PFNGLGENTEXTURES_PROC* glpfGenTextures = NULL;
PFNGLISTEXTURE_PROC* glpfIsTexture = NULL;
PFNGLPOLYGONOFFSET_PROC* glpfPolygonOffset = NULL;
PFNGLTEXSUBIMAGE1D_PROC* glpfTexSubImage1D = NULL;
PFNGLTEXSUBIMAGE2D_PROC* glpfTexSubImage2D = NULL;

/* GL_VERSION_1_2 */

PFNGLCOPYTEXSUBIMAGE3D_PROC* glpfCopyTexSubImage3D = NULL;
PFNGLDRAWRANGEELEMENTS_PROC* glpfDrawRangeElements = NULL;
PFNGLTEXIMAGE3D_PROC* glpfTexImage3D = NULL;
PFNGLTEXSUBIMAGE3D_PROC* glpfTexSubImage3D = NULL;

/* GL_VERSION_1_3 */

PFNGLACTIVETEXTURE_PROC* glpfActiveTexture = NULL;
PFNGLCOMPRESSEDTEXIMAGE1D_PROC* glpfCompressedTexImage1D = NULL;
PFNGLCOMPRESSEDTEXIMAGE2D_PROC* glpfCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3D_PROC* glpfCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1D_PROC* glpfCompressedTexSubImage1D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2D_PROC* glpfCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3D_PROC* glpfCompressedTexSubImage3D = NULL;
PFNGLGETCOMPRESSEDTEXIMAGE_PROC* glpfGetCompressedTexImage = NULL;
PFNGLSAMPLECOVERAGE_PROC* glpfSampleCoverage = NULL;

/* GL_VERSION_1_4 */

PFNGLBLENDCOLOR_PROC* glpfBlendColor = NULL;
PFNGLBLENDEQUATION_PROC* glpfBlendEquation = NULL;
PFNGLBLENDFUNCSEPARATE_PROC* glpfBlendFuncSeparate = NULL;
PFNGLMULTIDRAWARRAYS_PROC* glpfMultiDrawArrays = NULL;
PFNGLMULTIDRAWELEMENTS_PROC* glpfMultiDrawElements = NULL;
PFNGLPOINTPARAMETERF_PROC* glpfPointParameterf = NULL;
PFNGLPOINTPARAMETERFV_PROC* glpfPointParameterfv = NULL;
PFNGLPOINTPARAMETERI_PROC* glpfPointParameteri = NULL;
PFNGLPOINTPARAMETERIV_PROC* glpfPointParameteriv = NULL;

/* GL_VERSION_1_5 */

PFNGLBEGINQUERY_PROC* glpfBeginQuery = NULL;
PFNGLBINDBUFFER_PROC* glpfBindBuffer = NULL;
PFNGLBUFFERDATA_PROC* glpfBufferData = NULL;
PFNGLBUFFERSUBDATA_PROC* glpfBufferSubData = NULL;
PFNGLDELETEBUFFERS_PROC* glpfDeleteBuffers = NULL;
PFNGLDELETEQUERIES_PROC* glpfDeleteQueries = NULL;
PFNGLENDQUERY_PROC* glpfEndQuery = NULL;
PFNGLGENBUFFERS_PROC* glpfGenBuffers = NULL;
PFNGLGENQUERIES_PROC* glpfGenQueries = NULL;
PFNGLGETBUFFERPARAMETERIV_PROC* glpfGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERV_PROC* glpfGetBufferPointerv = NULL;
PFNGLGETBUFFERSUBDATA_PROC* glpfGetBufferSubData = NULL;
PFNGLGETQUERYOBJECTIV_PROC* glpfGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUIV_PROC* glpfGetQueryObjectuiv = NULL;
PFNGLGETQUERYIV_PROC* glpfGetQueryiv = NULL;
PFNGLISBUFFER_PROC* glpfIsBuffer = NULL;
PFNGLISQUERY_PROC* glpfIsQuery = NULL;
PFNGLMAPBUFFER_PROC* glpfMapBuffer = NULL;
PFNGLUNMAPBUFFER_PROC* glpfUnmapBuffer = NULL;

/* GL_VERSION_2_0 */

PFNGLATTACHSHADER_PROC* glpfAttachShader = NULL;
PFNGLBINDATTRIBLOCATION_PROC* glpfBindAttribLocation = NULL;
PFNGLBLENDEQUATIONSEPARATE_PROC* glpfBlendEquationSeparate = NULL;
PFNGLCOMPILESHADER_PROC* glpfCompileShader = NULL;
PFNGLCREATEPROGRAM_PROC* glpfCreateProgram = NULL;
PFNGLCREATESHADER_PROC* glpfCreateShader = NULL;
PFNGLDELETEPROGRAM_PROC* glpfDeleteProgram = NULL;
PFNGLDELETESHADER_PROC* glpfDeleteShader = NULL;
PFNGLDETACHSHADER_PROC* glpfDetachShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAY_PROC* glpfDisableVertexAttribArray = NULL;
PFNGLDRAWBUFFERS_PROC* glpfDrawBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAY_PROC* glpfEnableVertexAttribArray = NULL;
PFNGLGETACTIVEATTRIB_PROC* glpfGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORM_PROC* glpfGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERS_PROC* glpfGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATION_PROC* glpfGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOG_PROC* glpfGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIV_PROC* glpfGetProgramiv = NULL;
PFNGLGETSHADERINFOLOG_PROC* glpfGetShaderInfoLog = NULL;
PFNGLGETSHADERSOURCE_PROC* glpfGetShaderSource = NULL;
PFNGLGETSHADERIV_PROC* glpfGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATION_PROC* glpfGetUniformLocation = NULL;
PFNGLGETUNIFORMFV_PROC* glpfGetUniformfv = NULL;
PFNGLGETUNIFORMIV_PROC* glpfGetUniformiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERV_PROC* glpfGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBDV_PROC* glpfGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFV_PROC* glpfGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIV_PROC* glpfGetVertexAttribiv = NULL;
PFNGLISPROGRAM_PROC* glpfIsProgram = NULL;
PFNGLISSHADER_PROC* glpfIsShader = NULL;
PFNGLLINKPROGRAM_PROC* glpfLinkProgram = NULL;
PFNGLSHADERSOURCE_PROC* glpfShaderSource = NULL;
PFNGLSTENCILFUNCSEPARATE_PROC* glpfStencilFuncSeparate = NULL;
PFNGLSTENCILMASKSEPARATE_PROC* glpfStencilMaskSeparate = NULL;
PFNGLSTENCILOPSEPARATE_PROC* glpfStencilOpSeparate = NULL;
PFNGLUNIFORM1F_PROC* glpfUniform1f = NULL;
PFNGLUNIFORM1FV_PROC* glpfUniform1fv = NULL;
PFNGLUNIFORM1I_PROC* glpfUniform1i = NULL;
PFNGLUNIFORM1IV_PROC* glpfUniform1iv = NULL;
PFNGLUNIFORM2F_PROC* glpfUniform2f = NULL;
PFNGLUNIFORM2FV_PROC* glpfUniform2fv = NULL;
PFNGLUNIFORM2I_PROC* glpfUniform2i = NULL;
PFNGLUNIFORM2IV_PROC* glpfUniform2iv = NULL;
PFNGLUNIFORM3F_PROC* glpfUniform3f = NULL;
PFNGLUNIFORM3FV_PROC* glpfUniform3fv = NULL;
PFNGLUNIFORM3I_PROC* glpfUniform3i = NULL;
PFNGLUNIFORM3IV_PROC* glpfUniform3iv = NULL;
PFNGLUNIFORM4F_PROC* glpfUniform4f = NULL;
PFNGLUNIFORM4FV_PROC* glpfUniform4fv = NULL;
PFNGLUNIFORM4I_PROC* glpfUniform4i = NULL;
PFNGLUNIFORM4IV_PROC* glpfUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FV_PROC* glpfUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FV_PROC* glpfUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FV_PROC* glpfUniformMatrix4fv = NULL;
PFNGLUSEPROGRAM_PROC* glpfUseProgram = NULL;
PFNGLVALIDATEPROGRAM_PROC* glpfValidateProgram = NULL;
PFNGLVERTEXATTRIB1D_PROC* glpfVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DV_PROC* glpfVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1F_PROC* glpfVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FV_PROC* glpfVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1S_PROC* glpfVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SV_PROC* glpfVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2D_PROC* glpfVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DV_PROC* glpfVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2F_PROC* glpfVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FV_PROC* glpfVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2S_PROC* glpfVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SV_PROC* glpfVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3D_PROC* glpfVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DV_PROC* glpfVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3F_PROC* glpfVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FV_PROC* glpfVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3S_PROC* glpfVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SV_PROC* glpfVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBV_PROC* glpfVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIV_PROC* glpfVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSV_PROC* glpfVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUB_PROC* glpfVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBV_PROC* glpfVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIV_PROC* glpfVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSV_PROC* glpfVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BV_PROC* glpfVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4D_PROC* glpfVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DV_PROC* glpfVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4F_PROC* glpfVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FV_PROC* glpfVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IV_PROC* glpfVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4S_PROC* glpfVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SV_PROC* glpfVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBV_PROC* glpfVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIV_PROC* glpfVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USV_PROC* glpfVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBPOINTER_PROC* glpfVertexAttribPointer = NULL;

/* GL_VERSION_2_1 */

PFNGLUNIFORMMATRIX2X3FV_PROC* glpfUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4FV_PROC* glpfUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3X2FV_PROC* glpfUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4FV_PROC* glpfUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4X2FV_PROC* glpfUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3FV_PROC* glpfUniformMatrix4x3fv = NULL;

/* GL_VERSION_3_0 */

PFNGLBEGINCONDITIONALRENDER_PROC* glpfBeginConditionalRender = NULL;
PFNGLBEGINTRANSFORMFEEDBACK_PROC* glpfBeginTransformFeedback = NULL;
PFNGLBINDBUFFERBASE_PROC* glpfBindBufferBase = NULL;
PFNGLBINDBUFFERRANGE_PROC* glpfBindBufferRange = NULL;
PFNGLBINDFRAGDATALOCATION_PROC* glpfBindFragDataLocation = NULL;
PFNGLBINDFRAMEBUFFER_PROC* glpfBindFramebuffer = NULL;
PFNGLBINDRENDERBUFFER_PROC* glpfBindRenderbuffer = NULL;
PFNGLBINDVERTEXARRAY_PROC* glpfBindVertexArray = NULL;
PFNGLBLITFRAMEBUFFER_PROC* glpfBlitFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUS_PROC* glpfCheckFramebufferStatus = NULL;
PFNGLCLAMPCOLOR_PROC* glpfClampColor = NULL;
PFNGLCLEARBUFFERFI_PROC* glpfClearBufferfi = NULL;
PFNGLCLEARBUFFERFV_PROC* glpfClearBufferfv = NULL;
PFNGLCLEARBUFFERIV_PROC* glpfClearBufferiv = NULL;
PFNGLCLEARBUFFERUIV_PROC* glpfClearBufferuiv = NULL;
PFNGLCOLORMASKI_PROC* glpfColorMaski = NULL;
PFNGLDELETEFRAMEBUFFERS_PROC* glpfDeleteFramebuffers = NULL;
PFNGLDELETERENDERBUFFERS_PROC* glpfDeleteRenderbuffers = NULL;
PFNGLDELETEVERTEXARRAYS_PROC* glpfDeleteVertexArrays = NULL;
PFNGLDISABLEI_PROC* glpfDisablei = NULL;
PFNGLENABLEI_PROC* glpfEnablei = NULL;
PFNGLENDCONDITIONALRENDER_PROC* glpfEndConditionalRender = NULL;
PFNGLENDTRANSFORMFEEDBACK_PROC* glpfEndTransformFeedback = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGE_PROC* glpfFlushMappedBufferRange = NULL;
PFNGLFRAMEBUFFERRENDERBUFFER_PROC* glpfFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE1D_PROC* glpfFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE2D_PROC* glpfFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE3D_PROC* glpfFramebufferTexture3D = NULL;
PFNGLFRAMEBUFFERTEXTURELAYER_PROC* glpfFramebufferTextureLayer = NULL;
PFNGLGENFRAMEBUFFERS_PROC* glpfGenFramebuffers = NULL;
PFNGLGENRENDERBUFFERS_PROC* glpfGenRenderbuffers = NULL;
PFNGLGENVERTEXARRAYS_PROC* glpfGenVertexArrays = NULL;
PFNGLGENERATEMIPMAP_PROC* glpfGenerateMipmap = NULL;
PFNGLGETBOOLEANI_V_PROC* glpfGetBooleani_v = NULL;
PFNGLGETFRAGDATALOCATION_PROC* glpfGetFragDataLocation = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIV_PROC* glpfGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETINTEGERI_V_PROC* glpfGetIntegeri_v = NULL;
PFNGLGETRENDERBUFFERPARAMETERIV_PROC* glpfGetRenderbufferParameteriv = NULL;
PFNGLGETSTRINGI_PROC* glpfGetStringi = NULL;
PFNGLGETTEXPARAMETERIIV_PROC* glpfGetTexParameterIiv = NULL;
PFNGLGETTEXPARAMETERIUIV_PROC* glpfGetTexParameterIuiv = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYING_PROC* glpfGetTransformFeedbackVarying = NULL;
PFNGLGETUNIFORMUIV_PROC* glpfGetUniformuiv = NULL;
PFNGLGETVERTEXATTRIBIIV_PROC* glpfGetVertexAttribIiv = NULL;
PFNGLGETVERTEXATTRIBIUIV_PROC* glpfGetVertexAttribIuiv = NULL;
PFNGLISENABLEDI_PROC* glpfIsEnabledi = NULL;
PFNGLISFRAMEBUFFER_PROC* glpfIsFramebuffer = NULL;
PFNGLISRENDERBUFFER_PROC* glpfIsRenderbuffer = NULL;
PFNGLISVERTEXARRAY_PROC* glpfIsVertexArray = NULL;
PFNGLMAPBUFFERRANGE_PROC* glpfMapBufferRange = NULL;
PFNGLRENDERBUFFERSTORAGE_PROC* glpfRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLE_PROC* glpfRenderbufferStorageMultisample = NULL;
PFNGLTEXPARAMETERIIV_PROC* glpfTexParameterIiv = NULL;
PFNGLTEXPARAMETERIUIV_PROC* glpfTexParameterIuiv = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGS_PROC* glpfTransformFeedbackVaryings = NULL;
PFNGLUNIFORM1UI_PROC* glpfUniform1ui = NULL;
PFNGLUNIFORM1UIV_PROC* glpfUniform1uiv = NULL;
PFNGLUNIFORM2UI_PROC* glpfUniform2ui = NULL;
PFNGLUNIFORM2UIV_PROC* glpfUniform2uiv = NULL;
PFNGLUNIFORM3UI_PROC* glpfUniform3ui = NULL;
PFNGLUNIFORM3UIV_PROC* glpfUniform3uiv = NULL;
PFNGLUNIFORM4UI_PROC* glpfUniform4ui = NULL;
PFNGLUNIFORM4UIV_PROC* glpfUniform4uiv = NULL;
PFNGLVERTEXATTRIBI1I_PROC* glpfVertexAttribI1i = NULL;
PFNGLVERTEXATTRIBI1IV_PROC* glpfVertexAttribI1iv = NULL;
PFNGLVERTEXATTRIBI1UI_PROC* glpfVertexAttribI1ui = NULL;
PFNGLVERTEXATTRIBI1UIV_PROC* glpfVertexAttribI1uiv = NULL;
PFNGLVERTEXATTRIBI2I_PROC* glpfVertexAttribI2i = NULL;
PFNGLVERTEXATTRIBI2IV_PROC* glpfVertexAttribI2iv = NULL;
PFNGLVERTEXATTRIBI2UI_PROC* glpfVertexAttribI2ui = NULL;
PFNGLVERTEXATTRIBI2UIV_PROC* glpfVertexAttribI2uiv = NULL;
PFNGLVERTEXATTRIBI3I_PROC* glpfVertexAttribI3i = NULL;
PFNGLVERTEXATTRIBI3IV_PROC* glpfVertexAttribI3iv = NULL;
PFNGLVERTEXATTRIBI3UI_PROC* glpfVertexAttribI3ui = NULL;
PFNGLVERTEXATTRIBI3UIV_PROC* glpfVertexAttribI3uiv = NULL;
PFNGLVERTEXATTRIBI4BV_PROC* glpfVertexAttribI4bv = NULL;
PFNGLVERTEXATTRIBI4I_PROC* glpfVertexAttribI4i = NULL;
PFNGLVERTEXATTRIBI4IV_PROC* glpfVertexAttribI4iv = NULL;
PFNGLVERTEXATTRIBI4SV_PROC* glpfVertexAttribI4sv = NULL;
PFNGLVERTEXATTRIBI4UBV_PROC* glpfVertexAttribI4ubv = NULL;
PFNGLVERTEXATTRIBI4UI_PROC* glpfVertexAttribI4ui = NULL;
PFNGLVERTEXATTRIBI4UIV_PROC* glpfVertexAttribI4uiv = NULL;
PFNGLVERTEXATTRIBI4USV_PROC* glpfVertexAttribI4usv = NULL;
PFNGLVERTEXATTRIBIPOINTER_PROC* glpfVertexAttribIPointer = NULL;

/* GL_VERSION_3_1 */

PFNGLCOPYBUFFERSUBDATA_PROC* glpfCopyBufferSubData = NULL;
PFNGLDRAWARRAYSINSTANCED_PROC* glpfDrawArraysInstanced = NULL;
PFNGLDRAWELEMENTSINSTANCED_PROC* glpfDrawElementsInstanced = NULL;
PFNGLGETACTIVEUNIFORMBLOCKNAME_PROC* glpfGetActiveUniformBlockName = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIV_PROC* glpfGetActiveUniformBlockiv = NULL;
PFNGLGETACTIVEUNIFORMNAME_PROC* glpfGetActiveUniformName = NULL;
PFNGLGETACTIVEUNIFORMSIV_PROC* glpfGetActiveUniformsiv = NULL;
PFNGLGETUNIFORMBLOCKINDEX_PROC* glpfGetUniformBlockIndex = NULL;
PFNGLGETUNIFORMINDICES_PROC* glpfGetUniformIndices = NULL;
PFNGLPRIMITIVERESTARTINDEX_PROC* glpfPrimitiveRestartIndex = NULL;
PFNGLTEXBUFFER_PROC* glpfTexBuffer = NULL;
PFNGLUNIFORMBLOCKBINDING_PROC* glpfUniformBlockBinding = NULL;

/* GL_VERSION_3_2 */

PFNGLCLIENTWAITSYNC_PROC* glpfClientWaitSync = NULL;
PFNGLDELETESYNC_PROC* glpfDeleteSync = NULL;
PFNGLDRAWELEMENTSBASEVERTEX_PROC* glpfDrawElementsBaseVertex = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEX_PROC* glpfDrawElementsInstancedBaseVertex = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEX_PROC* glpfDrawRangeElementsBaseVertex = NULL;
PFNGLFENCESYNC_PROC* glpfFenceSync = NULL;
PFNGLFRAMEBUFFERTEXTURE_PROC* glpfFramebufferTexture = NULL;
PFNGLGETBUFFERPARAMETERI64V_PROC* glpfGetBufferParameteri64v = NULL;
PFNGLGETINTEGER64I_V_PROC* glpfGetInteger64i_v = NULL;
PFNGLGETINTEGER64V_PROC* glpfGetInteger64v = NULL;
PFNGLGETMULTISAMPLEFV_PROC* glpfGetMultisamplefv = NULL;
PFNGLGETSYNCIV_PROC* glpfGetSynciv = NULL;
PFNGLISSYNC_PROC* glpfIsSync = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEX_PROC* glpfMultiDrawElementsBaseVertex = NULL;
PFNGLPROVOKINGVERTEX_PROC* glpfProvokingVertex = NULL;
PFNGLSAMPLEMASKI_PROC* glpfSampleMaski = NULL;
PFNGLTEXIMAGE2DMULTISAMPLE_PROC* glpfTexImage2DMultisample = NULL;
PFNGLTEXIMAGE3DMULTISAMPLE_PROC* glpfTexImage3DMultisample = NULL;
PFNGLWAITSYNC_PROC* glpfWaitSync = NULL;

/* GL_VERSION_3_3 */

PFNGLBINDFRAGDATALOCATIONINDEXED_PROC* glpfBindFragDataLocationIndexed = NULL;
PFNGLBINDSAMPLER_PROC* glpfBindSampler = NULL;
PFNGLDELETESAMPLERS_PROC* glpfDeleteSamplers = NULL;
PFNGLGENSAMPLERS_PROC* glpfGenSamplers = NULL;
PFNGLGETFRAGDATAINDEX_PROC* glpfGetFragDataIndex = NULL;
PFNGLGETQUERYOBJECTI64V_PROC* glpfGetQueryObjecti64v = NULL;
PFNGLGETQUERYOBJECTUI64V_PROC* glpfGetQueryObjectui64v = NULL;
PFNGLGETSAMPLERPARAMETERIIV_PROC* glpfGetSamplerParameterIiv = NULL;
PFNGLGETSAMPLERPARAMETERIUIV_PROC* glpfGetSamplerParameterIuiv = NULL;
PFNGLGETSAMPLERPARAMETERFV_PROC* glpfGetSamplerParameterfv = NULL;
PFNGLGETSAMPLERPARAMETERIV_PROC* glpfGetSamplerParameteriv = NULL;
PFNGLISSAMPLER_PROC* glpfIsSampler = NULL;
PFNGLQUERYCOUNTER_PROC* glpfQueryCounter = NULL;
PFNGLSAMPLERPARAMETERIIV_PROC* glpfSamplerParameterIiv = NULL;
PFNGLSAMPLERPARAMETERIUIV_PROC* glpfSamplerParameterIuiv = NULL;
PFNGLSAMPLERPARAMETERF_PROC* glpfSamplerParameterf = NULL;
PFNGLSAMPLERPARAMETERFV_PROC* glpfSamplerParameterfv = NULL;
PFNGLSAMPLERPARAMETERI_PROC* glpfSamplerParameteri = NULL;
PFNGLSAMPLERPARAMETERIV_PROC* glpfSamplerParameteriv = NULL;
PFNGLVERTEXATTRIBDIVISOR_PROC* glpfVertexAttribDivisor = NULL;
PFNGLVERTEXATTRIBP1UI_PROC* glpfVertexAttribP1ui = NULL;
PFNGLVERTEXATTRIBP1UIV_PROC* glpfVertexAttribP1uiv = NULL;
PFNGLVERTEXATTRIBP2UI_PROC* glpfVertexAttribP2ui = NULL;
PFNGLVERTEXATTRIBP2UIV_PROC* glpfVertexAttribP2uiv = NULL;
PFNGLVERTEXATTRIBP3UI_PROC* glpfVertexAttribP3ui = NULL;
PFNGLVERTEXATTRIBP3UIV_PROC* glpfVertexAttribP3uiv = NULL;
PFNGLVERTEXATTRIBP4UI_PROC* glpfVertexAttribP4ui = NULL;
PFNGLVERTEXATTRIBP4UIV_PROC* glpfVertexAttribP4uiv = NULL;


#ifdef __cplusplus
}
#endif
