#pragma once

#include <memory>
#include <vector>

#include <engine/render/Window.hpp>
#include <engine/utils/Flags.hpp>

namespace spdlog {
class logger;
}

namespace expengine {

enum class AttachmentsFlagBits : uint32_t
{
    eColorAttachment = 1,
    eDepthAttachments = 1 << 1
};
using AttachmentsFlags = Flags<AttachmentsFlagBits>;

template <> struct FlagTraits<AttachmentsFlagBits> {
    enum : uint32_t
    {
        allFlags = uint32_t(AttachmentsFlagBits::eColorAttachment)
            | uint32_t(AttachmentsFlagBits::eDepthAttachments)
    };
};

namespace render {

class RenderingContext {
public:
    virtual ~RenderingContext() = default;

    /* Accessors */
    virtual inline const Window& window() const = 0;

    /* Call to make the RenderingContext check its surface and adapt its objects to
     * it. */
    virtual void handleSurfaceChanges() = 0;
    /* Frame rendering */
    virtual void beginFrame() = 0;
    virtual void submitFrame() = 0;

    virtual std::shared_ptr<RenderingContext> clone(
        std::shared_ptr<Window> window,
        AttachmentsFlags attachmentFlags)
        = 0;

protected:
    RenderingContext();

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
