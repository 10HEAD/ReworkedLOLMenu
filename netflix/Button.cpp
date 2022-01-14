#include "pch.h"

#include "Button.h"
#include <iostream>
#include "Menu.h"

Button::Button(std::string text, int status)
{
    this->text = text;
    this->status = status;
}

void Button::Print()
{
    std::cout << this->text;
    if (status != -1) {

        std::cout << ansi::foreground_white << " [";
        if (status == 0) {
            std::cout << ansi::foreground_red << "OFF";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 1) {
            std::cout << ansi::foreground_yellow << "LOADING";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 2) {
            std::cout << ansi::foreground_green << "ON";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }

        //added more for status
        else if (status == 3) {
            std::cout << ansi::foreground_red << "RED";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 4) {
            std::cout << ansi::foreground_yellow << "YELLOW";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 5) {
            std::cout << ansi::foreground_purple << "PURPLE";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 6) {
            std::cout << ansi::foreground_green << "SAVED";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 7) {
            std::cout << ansi::foreground_cyan << "HEAD";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 8) {
            std::cout << ansi::foreground_cyan << "NECK";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 9) {
            std::cout << ansi::foreground_cyan << "CHEST";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 10) {
            std::cout << ansi::foreground_cyan << "LEFT CLICK";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 11) {
            std::cout << ansi::foreground_cyan << "MB4";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 12) {
            std::cout << ansi::foreground_cyan << "MB5";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
        else if (status == 13) {
            std::cout << ansi::foreground_red << "NOT SET";
            std::cout << ansi::reset << ansi::foreground_white << "]" << ansi::reset;
        }
    }
    std::cout << std::endl;
}

void Button::UpdateStatus()
{
    if (this->status == 2)
        this->status = 0;
    else if (this->status == 0) {
        this->status = 1;
    }
    else if (this->status == 1)
        this->status = 2;
}

void Button::UpdateStatus(int status)
{
    this->status = status;
}

void Button::UpdateStatus(bool status)
{
    int iStatus = status ? 2 : 0;
    this->status = iStatus;
}