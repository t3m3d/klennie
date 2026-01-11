#pragma once

#include "system.hpp"

class UI
{
public:
    UI();
    ~UI();

    void run();

private:
    System system_;

    void draw();
};