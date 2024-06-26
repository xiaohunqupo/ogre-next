/*
-----------------------------------------------------------------------------
This source file is part of OGRE-Next
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef _OgreMetalTextureGpu_H_
#define _OgreMetalTextureGpu_H_

#include "OgreMetalPrerequisites.h"

#include "OgreTextureGpu.h"

#include "OgreDescriptorSetTexture.h"
#include "OgreDescriptorSetUav.h"

#import <Metal/MTLTexture.h>

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class _OgreMetalExport MetalTextureGpu : public TextureGpu
    {
    protected:
        /// This will not be owned by us if hasAutomaticBatching is true.
        /// It will also not be owned by us if we're not in GpuResidency::Resident
        /// This will always point to:
        ///     * A GL texture owned by us.
        ///     * A 4x4 dummy texture (now owned by us).
        ///     * A 64x64 mipmapped texture of us (but now owned by us).
        ///     * A GL texture not owned by us, but contains the final information.
        id<MTLTexture> mDisplayTextureName;

        /// When we're transitioning to GpuResidency::Resident but we're not there yet,
        /// we will be either displaying a 4x4 dummy texture or a 64x64 one. However
        /// we reserve a spot to a final place will already be there for us once the
        /// texture data is fully uploaded. This variable contains that texture.
        /// Async upload operations should stack to this variable.
        /// May contain:
        ///     1. 0, if not resident or resident but not yet reached main thread.
        ///     2. The texture
        ///     3. An msaa texture (hasMsaaExplicitResolves == true)
        ///     4. The msaa resolved texture (hasMsaaExplicitResolves==false)
        id<MTLTexture> mFinalTextureName;
        /// Only used when hasMsaaExplicitResolves() == false.
        id<MTLTexture> mMsaaFramebufferName;

        void createInternalResourcesImpl() override;
        void destroyInternalResourcesImpl() override;

        MTLTextureType getMetalTextureType() const;

    public:
        MetalTextureGpu( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy, VaoManager *vaoManager,
                         IdString name, uint32 textureFlags, TextureTypes::TextureTypes initialType,
                         TextureGpuManager *textureManager );
        ~MetalTextureGpu() override;

        void setTextureType( TextureTypes::TextureTypes textureType ) override;

        void copyTo(
            TextureGpu *dst, const TextureBox &dstBox, uint8 dstMipLevel, const TextureBox &srcBox,
            uint8 srcMipLevel, bool keepResolvedTexSynced = true,
            CopyEncTransitionMode::CopyEncTransitionMode srcTransitionMode = CopyEncTransitionMode::Auto,
            CopyEncTransitionMode::CopyEncTransitionMode dstTransitionMode =
                CopyEncTransitionMode::Auto ) override;

        void _autogenerateMipmaps( CopyEncTransitionMode::CopyEncTransitionMode transitionMode =
                                       CopyEncTransitionMode::Auto ) override;

        void getSubsampleLocations( vector<Vector2>::type locations ) override;

        void notifyDataIsReady() override;
        bool _isDataReadyImpl() const override;

        void _setToDisplayDummyTexture() override;
        void _notifyTextureSlotChanged( const TexturePool *newPool, uint16 slice ) override;

        id<MTLTexture> getView( PixelFormatGpu pixelFormat, uint8 mipLevel, uint8 numMipmaps,
                                uint16 arraySlice, bool cubemapsAs2DArrays, bool forUav );
        id<MTLTexture> getView( DescriptorSetTexture2::TextureSlot texSlot );
        id<MTLTexture> getView( DescriptorSetUav::TextureSlot uavSlot );

        id<MTLTexture> getDisplayTextureName() const { return mDisplayTextureName; }
        id<MTLTexture> getFinalTextureName() const { return mFinalTextureName; }
        id<MTLTexture> getMsaaFramebufferName() const { return mMsaaFramebufferName; }

        virtual void getCustomAttribute( IdString name, void *pData ) override;
    };

    class _OgreMetalExport MetalTextureGpuRenderTarget : public MetalTextureGpu
    {
    protected:
        uint16         mDepthBufferPoolId;
        bool           mPreferDepthTexture;
        PixelFormatGpu mDesiredDepthBufferFormat;
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        OrientationMode mOrientationMode;
#endif

    public:
        MetalTextureGpuRenderTarget( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy,
                                     VaoManager *vaoManager, IdString name, uint32 textureFlags,
                                     TextureTypes::TextureTypes initialType,
                                     TextureGpuManager         *textureManager );

        void           _setDepthBufferDefaults( uint16 depthBufferPoolId, bool preferDepthTexture,
                                                PixelFormatGpu desiredDepthBufferFormat ) override;
        uint16         getDepthBufferPoolId() const override;
        bool           getPreferDepthTexture() const override;
        PixelFormatGpu getDesiredDepthBufferFormat() const override;

        void setOrientationMode( OrientationMode orientationMode ) override;
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        OrientationMode getOrientationMode() const override;
#endif
    };
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
