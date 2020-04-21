#pragma once

namespace dang::gl
{

class Window;

/// <summary>A context, which exists not per binding point, but per object type.</summary>
class ObjectContext {
public:
    /// <summary>Initializes the object context with the given window context.</summary>
    ObjectContext(Window& window);
    virtual ~ObjectContext() = default;

    /// <summary>Returns the associated window.</summary>
    Window& window() const;

private:
    Window& window_;
};

}
