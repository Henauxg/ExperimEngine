#pragma once

#include <memory>
#include <vector>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>
#include <engine/utils/Flags.hpp>

namespace expengine {
namespace render {

enum class AttachmentsFlagBits : uint32_t
{
    eColorAttachment = 1,
    eDepthAttachments = 1 << 1
};
using AttachmentsFlags = Flags<AttachmentsFlagBits>;

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
    RenderingContext()
        : logger_(spdlog::get(LOGGER_NAME)) {};

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
