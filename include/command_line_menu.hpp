// The "Command Line Menu" library, in c++.
//
// Webs: https://github.com/JaderoChan/CommandLineMenu
// You can contact me by email: c_dl_cn@outlook.com
//
// MIT License
//
// Copyright (c) 2024 頔珞JaderoChan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef COMMAND_LINE_MENU_HPP
#define COMMAND_LINE_MENU_HPP

#include <cstddef>      // size_t
#include <cstdlib>      // system()
#include <conio.h>      // _getch()
#include <string>       // string
#include <array>        // array
#include <vector>       // vector
#include <atomic>       // atomic
#include <iostream>     // cout, endl, flush, getchar()
#include <stdexcept>    // runtime_error

class CommandLineMenu
{
public:
    using Rgb           = std::array<int, 3>;
    using VoidFunc      = void (*)();
    using Arg           = void*;
    using ArgFunc       = void (*)(Arg);

    static CommandLineMenu& getInstance()
    {
        static CommandLineMenu instance;
        return instance;
    }

    // @brief Add a new option to the last position.
    // @param optionText        The text of the option.
    // @param callbackFunc      The callback function when the option is triggered.
    // @param enableNewPage     Whether go to the new page when the option be triggered.
    void addOption(const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, callbackFunc });
    }

    // overload
    // @brief Add a new option to the last position.
    // @param optionText        The text of the option.
    // @param callbackFunc      The callback function when the option is triggered.
    // @param arg               The argument of the callback function.
    // @param enableNewPage     Whether go to the new page when the option be triggered.
    void addOption(const std::string& optionText, ArgFunc callbackFunc, Arg arg, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });
    }

    // @brief Insert a new option to the specified position.
    // @param index             The position to insert the option.
    // @param optionText        The text of the option.
    // @param callbackFunc      The callback function when the option is triggered.
    // @param enableNewPage     Whether go to the new page when the option be triggered.
    void insertOption(size_t index, const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index, Option { enableNewPage, optionText, callbackFunc });
    }

    // @brief Insert a new option to the specified position.
    // @param index             The position to insert the option.
    // @param optionText        The text of the option.
    // @param callbackFunc      The callback function when the option is triggered.
    // @param arg               The argument of the callback function.
    // @param enableNewPage     Whether go to the new page when the option be triggered.
    void insertOption(size_t index, const std::string& optionText, ArgFunc callbackFunc, Arg arg,
                      bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index,
                        Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });
    }

    // @brief Remove an option by its index.
    void removeOption(size_t index) { options_.erase(options_.begin() + index); }

    // @brief Remove all options.
    void removeAllOption() { options_.clear(); }

    // @brief Set the whether go to the new page when the option be triggered.
    void setOptionEnableNewPage(size_t index, bool enable) { options_[index].enableNewPage = enable; }

    // @brief Set the text of specified option.
    void setOptionText(size_t index, const std::string& text) { options_[index].text = text; }

    // @brief Set the callback function of specified option.
    void setOptionCallback(size_t index, VoidFunc callbackFunc)
    {
        options_[index].callback = CallbackFunc(callbackFunc);
    }

    // @brief Set the callback function and argument of specified option.
    void setOptionCallback(size_t index, ArgFunc callbackFunc, Arg arg)
    {
        options_[index].callback = CallbackFunc(callbackFunc, arg);
    }

    // @brief Set the argument of specified option.
    // @attention Only available when the option has argument.
    // @throw runtime_error If the option has no callback function with argument.
    void setOptionCallbackArg(size_t index, Arg arg)
    {
        if (options_[index].callback.isArgFunc)
            options_[index].callback.argFuncArg.second = arg;
        else
            throw std::runtime_error("Specified option has no callback function with argument.");
    }

    // @brief Get the count of options.
    size_t optionCount() const { return options_.size(); }

    // @brief Set whether to show the index of each option.
    void setEnableShowIndex(bool enable) { enableShowIndex_ = enable; }

    // @brief Set whether to show the title (option text) of each option page.
    void setEnableShowOptionPageTitle(bool enable) { enableShowOptionPageTitle_ = enable; }

    // @brief Set the max column of option menu, used to align the output.
    void setMaxColumn(size_t maxColumn) { maxColumn_ = maxColumn == 0 ? 1 : maxColumn; }

    // @brief Set the current selected option (highlight option).
    void setHighlightedOption(size_t index) { selectedOption_ = index; }

    // @brief Select the specified option, the same as setHighlightedOption().
    void selectOption(size_t index) { setHighlightedOption(index); }

    // @brief Set the enter key, used to trigger the selected option.
    void setEnterKey(int key) { enterKey_ = key; }

    // @brief Set the esc key, used to return to the main menu or exit the input loop.
    void setEscKey(int key) { escKey_ = key; }

    // @brief Set the directional control key, used to select option.
    void setDirectionalControlKey(int left, int up, int right, int down)
    {
        directionalControlKey_ = { left, up, right, down };
    }

    // @overload
    void setDirectionalControlKey(const std::array<int, 4>& keys) { directionalControlKey_ = keys; }

    // @brief Set the background color of the option text.
    void setBackgroundColor(int r, int g, int b) { backgroundColor_ = { r, g, b }; }

    // @brief Set the foreground color of the option text.
    void setForegroundColor(int r, int g, int b) { foregroundColor_ = { r, g, b }; }

    // @brief Set the highlight background color of option selected.
    void setHighlightBackgroundColor(int r, int g, int b) { highlightBackgroundColor_ = { r, g, b }; }

    // @brief Set the highlight foreground color of option selected.
    void setHighlightForegroundColor(int r, int g, int b) { highlightForegroundColor_ = { r, g, b }; }

    // @brief Set the top text of the option list.
    void setTopText(const std::string& text) { topText_ = text; }

    // @brief Set the bottom text of the option list.
    void setBottomText(const std::string& text) { bottomText_ = text; }

    // @brief Set the text be displayed when the option page ended.
    // Example you can set "Press ESC key to back to the main menu."
    void setNewPageEndedText(const std::string& text) { newPageEndedText_ = text; }

    // @brief Select and trigger the specified option.
    // @note Not throw exception even if the index is out of range or the option's callback function is null.
    void triggerOption(size_t index)
    {
        if (index >= options_.size())
            return;

        selectOption(index);

        if (!options_[index].callback.isValid())
            return;

        if (options_[index].enableNewPage) {
            clearConsole();

            if (enableShowOptionPageTitle_) {
                if (enableShowIndex_)
                    std::cout << "[" << selectedOption_ << "] ";

                std::cout << options_[selectedOption_].text << '\n' << std::endl;
            }
        }

        options_[selectedOption_].callback.execute();

        if (!newPageEndedText_.empty())
            std::cout << '\n' << newPageEndedText_ << std::endl;

        while (::_getch() != escKey_)
            continue;

        clearConsole();
    }

    // @brief Clear the console.
    void clearConsole()
    {
#ifdef _WIN32
        ::system("cls");
#else
        ::system("clear");
#endif // _WIN32
    }

    // @brief Output all options to console.
    void show()
    {
        clearConsole();
        update_();
    }

    // @brief Start to recving input from console.
    // @note This function will block the current thread, and will not return until the input loop is exited.
    void startReceiveInput()
    {
        while (!shouldEndReceiveInput_) {
            // Clear the input buffer.
            std::cin.clear();

            int key = ::_getch();

            if (key == enterKey_) {
                triggerOption(selectedOption_);
                update_();
            } else if (key == escKey_) {
                shouldEndReceiveInput_ = true;
            } else if (key == directionalControlKey_[0]) {
                if (selectedOption_ > 0) {
                    selectOption(selectedOption_ - 1);
                    update_();
                }
            } else if (key == directionalControlKey_[1]) {
                size_t currentRow = selectedOption_ / maxColumn_;
                if (currentRow > 0) {
                    selectOption(selectedOption_ - maxColumn_);
                    update_();
                }
            } else if (key == directionalControlKey_[2]) {
                if (selectedOption_ < options_.size() - 1) {
                    selectOption(selectedOption_ + 1);
                    update_();
                }
            } else if (key == directionalControlKey_[3]) {
                size_t currentRow = selectedOption_ / maxColumn_;
                if (currentRow < options_.size() / maxColumn_) {
                    size_t expectedPos = selectedOption_ + maxColumn_;
                    expectedPos = expectedPos < options_.size() ? expectedPos : options_.size() - 1;

                    if (expectedPos != selectedOption_) {
                        selectOption(expectedPos);
                        update_();
                    }
                }
            }
        }
    }

    // @brief End the input loop.
    // @note This function is thread-safe.
    void endReceiveInput() { shouldEndReceiveInput_ = true; }

private:
    struct CallbackFunc
    {
        CallbackFunc(VoidFunc voidFunc) : isArgFunc(false), voidFunc(voidFunc) {}

        CallbackFunc(ArgFunc argFunc, Arg arg) : isArgFunc(true), argFuncArg(argFunc, arg) {}

        CallbackFunc(const CallbackFunc& other) : isArgFunc(other.isArgFunc)
        {
            if (isArgFunc)
                argFuncArg = other.argFuncArg;
            else
                voidFunc = other.voidFunc;
        }

        CallbackFunc& operator=(const CallbackFunc& other)
        {
            isArgFunc = other.isArgFunc;

            if (isArgFunc)
                argFuncArg = other.argFuncArg;
            else
                voidFunc = other.voidFunc;

            return *this;
        }

        bool isValid() const
        {
            if (isArgFunc)
                return argFuncArg.first;
            else
                return voidFunc;
        }

        void execute() const
        {
            if (isArgFunc)
                argFuncArg.first(argFuncArg.second);
            else
                voidFunc();
        }

        bool isArgFunc;
        union
        {
            VoidFunc voidFunc;
            std::pair<ArgFunc, Arg> argFuncArg;
        };
    };

    struct Option
    {
        bool enableNewPage;
        std::string text;
        CallbackFunc callback;
    };

    CommandLineMenu() : shouldEndReceiveInput_(false) {};

    CommandLineMenu(const CommandLineMenu&) = delete;

    CommandLineMenu& operator=(const CommandLineMenu&) = delete;

    bool isVaildColor_(int r, int g, int b) const
    {
        return (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255);
    }

    // @brief Reset all console attributes.
    void resetConsoleAttribute_() { std::cout << "\033[0m"; }

    // @brief Set the console background color of text.
    void setConsoleBackgroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";
    }

    // @brief Set the console foreground color of text.
    void setConsoleForegroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    }

    // @brief Output the text with specified color.
    void outputText_(const std::string& text, const Rgb& foregroundColor, const Rgb& backgroundColor)
    {
        setConsoleForegroundColor_(foregroundColor[0], foregroundColor[1], foregroundColor[2]);
        setConsoleBackgroundColor_(backgroundColor[0], backgroundColor[1], backgroundColor[2]);

        std::cout << text;

        resetConsoleAttribute_();
    }

    // @brief Update the console output.
    void update_()
    {
        std::cout << "\033[3J\033[H";

        if (!topText_.empty())
            std::cout << topText_ << '\n' << std::endl;

        for (size_t i = 0; i < options_.size(); ++i) {
            if (enableShowIndex_)
                std::cout << "[" << i << "] ";

            if (i == selectedOption_) {
                outputText_(options_[i].text, highlightForegroundColor_, highlightBackgroundColor_);
            } else {
                outputText_(options_[i].text, foregroundColor_, backgroundColor_);
            }

            if (maxColumn_ == 0 || i % maxColumn_ == maxColumn_ - 1 || i == options_.size() - 1)
                std::cout << std::endl;
            else
                std::cout << '\t';
        }

        if (!bottomText_.empty())
            std::cout << '\n' << bottomText_ << std::endl;

        std::cout << std::endl << std::flush;
    }

    // Whether to show the index of each option.
    bool enableShowIndex_                       = false;
    // Whether to show the title (option text) at top of option page.
    bool enableShowOptionPageTitle_             = false;
    // The max column of option list, used to align the output.
    // Default value is 1, and value 0 is same to value 1.
    size_t maxColumn_                           = 1;
    // Current selected option index.
    size_t selectedOption_                      = 0;
    // Enter key, used to trigger the option.
    int enterKey_                               = 0x0D;
    // Esc key, used to return to the main menu or exit the input loop.
    int escKey_                                 = 0x1B;
    // Directional control key, used to select option.
    // Left, Up, Right, Down
    std::array<int, 4> directionalControlKey_   = { 'a', 'w', 'd', 's' };
    // Default colir is invaid, indicating that do not set color.
    Rgb backgroundColor_                        = { -1, -1, -1 };
    Rgb foregroundColor_                        = { -1, -1, -1 };
    Rgb highlightBackgroundColor_               = { -1, -1, -1 };
    Rgb highlightForegroundColor_               = { 0, 255, 0 };
    std::string topText_;
    std::string bottomText_;
    std::string newPageEndedText_;
    std::vector<Option> options_;
    // Whether to end the input loop.
    std::atomic<bool> shouldEndReceiveInput_;
};

#endif // !COMMAND_LINE_MENU_HPP