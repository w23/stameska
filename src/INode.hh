#pragma once

#include <string>
#include <functional>

class INode {
public:
    virtual ~INode() noexcept {}

    // Alive while you have ptr to INode
    const std::string& getClassName() const noexcept { return class_name_; }
    const std::string& getName() const noexcept { return name_; }

    void setName(std::string name) { name_ = name; }

    virtual void visitChildren(const std::function<void(INode*)>& visitor) noexcept {}
    virtual void doUi() noexcept {}

protected:
    INode(const std::string& class_name, std::string name) : class_name_(class_name), name_(name) {}
    const std::string& class_name_;
    std::string name_;
};
