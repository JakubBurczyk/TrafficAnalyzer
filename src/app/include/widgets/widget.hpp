#pragma once

namespace Traffic{

class Widget{
    protected:
        bool hidden_ = false;
    public:
        virtual ~Widget() = default;

        bool is_hidden() { return hidden_; }
        void hide() { hidden_ = true; }
        void show() { hidden_ = false; }

        void set_hidden_state(bool state) { hidden_ = state; }

        virtual void gui() = 0;
};

} // namespace Traffic



