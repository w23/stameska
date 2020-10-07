#pragma once
#include <functional>

class INode {
public:
    virtual ~INode() noexcept {}
    virtual void visitChildren(std::function<void(INode*)> visitor) noexcept = 0;

    // Alive while you have ptr to INode
    const std::string& getClassName() const noexcept { return class_name_; }
    const std::string& getName() const noexcept { return name_; }

    void setName(std::string name) { name_ = name; }
protected:
    INode(const std::string& class_name, std::string name) : class_name_(class_name), name_(name) {}
    const std::string& class_name_;
    std::string name_;
};