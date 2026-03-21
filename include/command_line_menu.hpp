// The "Command Line Menu" library, in c++.
//
// Repository: https://github.com/JaderoChan/CommandLineMenu
// Contact email: c_dl_cn@outlook.com

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
#include <array>        // array
#include <atomic>       // atomic
#include <stdexcept>    // runtime_error
#include <iostream>     // cout, endl
#include <string>       // string
#include <vector>       // vector

#ifdef _WIN32
    #include <conio.h>  // _getch()
#else
    #include <termios.h>
    #include <unistd.h>
#endif // _WIN32

class CommandLineMenu
{
public:
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    using Rgb = std::array<int, 3>;
#else
    /**
     * @brief ANSI 256 color.
     * @note 0-15: Base 16 color. 16-231: 6x6x6 RGB. 232-255: Gray step.
     */
    using Rgb = int;

    enum Color16
    {
        COLOR_NONE = -1,
        COLOR_BLACK = 0,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE,
        COLOR_GRAY = COLOR_WHITE,
        COLOR_LIGHT_BLACK,
        COLOR_LIGHT_RED,
        COLOR_LIGHT_GREEN,
        COLOR_LIGHT_YELLOW,
        COLOR_LIGHT_BLUE,
        COLOR_LIGHT_MAGENTA,
        COLOR_LIGHT_CYAN,
        COLOR_LIGHT_WHITE
    };
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    using VoidFunc      = void (*)();
    using Arg           = void*;
    using ArgFunc       = void (*)(Arg);

    CommandLineMenu() : shouldEndReceiveInput_(false) {};

    ~CommandLineMenu() = default;

    CommandLineMenu(const CommandLineMenu& other) = delete;

    CommandLineMenu& operator=(const CommandLineMenu& other) = delete;

    static int getkey()
    {
    #ifdef _WIN32
        return ::_getch();
    #else
        struct termios oldAttr, newAttr;

        tcgetattr(STDIN_FILENO, &oldAttr);

        newAttr = oldAttr;
        newAttr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);

        int ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);

        return ch;
    #endif // _WIN32
    }

    /// @brief Get the number of options in the menu.
    size_t getOptionCount() const { return options_.size(); }

    std::string getOptionText(size_t index) const { return options_.at(index).text; }

    std::string getTopText() const { return topText_; }

    std::string getBottomText() const { return bottomText_; }

    /// @brief Add a new option to the end of the menu.
    /// @param optionText       The text displayed for the option.
    /// @param callbackFunc     The callback function to execute when the option is selected.
    /// @param enableNewPage    Whether to clear the console before executing the callback.
    void addOption(const std::string& optionText, VoidFunc callbackFunc,
        bool enableNewPage = true, bool waitKeyAfterEnd = true)
    {
        options_.push_back(Option { enableNewPage, waitKeyAfterEnd, optionText, callbackFunc });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief Add a new option to the end of the menu.
    /// @param optionText       The text displayed for the option.
    /// @param callbackFunc     The callback function with an argument.
    /// @param arg              The argument to pass to the callback function.
    /// @param enableNewPage    Whether to clear the console before executing the callback.
    void addOption(const std::string& optionText, ArgFunc callbackFunc, Arg arg,
        bool enableNewPage = true, bool waitKeyAfterEnd = true)
    {
        options_.push_back(Option { enableNewPage, waitKeyAfterEnd, optionText, CallbackFunc(callbackFunc, arg) });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @brief Insert a new option at the specified position.
    /// @param index            The position to insert the option (0-based).
    /// @param optionText       The text displayed for the option.
    /// @param callbackFunc     The callback function to execute when the option is selected.
    /// @param enableNewPage    Whether to clear the console before executing the callback.
    void insertOption(size_t index, const std::string& optionText, VoidFunc callbackFunc,
        bool enableNewPage = true, bool waitKeyAfterEnd = true)
    {
        options_.insert(options_.begin() + index, Option { enableNewPage, waitKeyAfterEnd, optionText, callbackFunc });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief Insert a new option at the specified position.
    /// @param index            The position to insert the option (0-based).
    /// @param optionText       The text displayed for the option.
    /// @param callbackFunc     The callback function with an argument.
    /// @param arg              The argument to pass to the callback function.
    /// @param enableNewPage    Whether to clear the console before executing the callback.
    void insertOption(size_t index, const std::string& optionText, ArgFunc callbackFunc, Arg arg,
          bool enableNewPage = true, bool waitKeyAfterEnd = true)
    {
        options_.insert(options_.begin() + index,
            Option { enableNewPage, waitKeyAfterEnd, optionText, CallbackFunc(callbackFunc, arg) });

        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @brief Remove an option by its index.
    void removeOption(size_t index) { options_.erase(options_.begin() + index); }

    /// @brief Remove all options.
    void removeAllOption()
    {
        options_.clear();
        if (enableAutoAdjustOptionTextWidth_)
            optionTextWidth_ = 0;
    }

    /// @brief Enable or disable console clearing for the specified option.
    void setOptionEnableNewPage(size_t index, bool enable) { options_[index].enableNewPage = enable; }

    /// @brief Enable or disable whether to wait for any key input to return to the main menu when the function ends.
    void setOptionWaitKeyAfterEnd(size_t index, bool enable) { options_[index].waitKeyAfterEnd = enable; }

    /// @brief Set the display text for the specified option.
    void setOptionText(size_t index, const std::string& text)
    {
        options_[index].text = text;
        if (enableAutoAdjustOptionTextWidth_ && text.size() > optionTextWidth_)
            optionTextWidth_ = text.size();
    }

    /// @brief Set the callback function for the specified option.
    void setOptionCallback(size_t index, VoidFunc callbackFunc)
    {
        options_[index].callback = CallbackFunc(callbackFunc);
    }

    /// @overload
    /// @brief Set the callback function and argument for the specified option.
    void setOptionCallback(size_t index, ArgFunc callbackFunc, Arg arg)
    {
        options_[index].callback = CallbackFunc(callbackFunc, arg);
    }

    /// @brief Set the argument for the specified option's callback function.
    /// @attention Only available for options with argument-based callbacks.
    /// @throw Throws std::runtime_error if the option does not have an argument-based callback.
    void setOptionCallbackArg(size_t index, Arg arg)
    {
        if (options_[index].callback.isArgFunc)
            options_[index].callback.argFuncArg.second = arg;
        else
            throw std::runtime_error("Specified option has no callback function with argument.");
    }

    /// @brief Enable or disable index display for each option.
    void setEnableShowIndex(bool enable) { enableShowIndex_ = enable; }

    /// @brief Enable or disable automatic adjustment of option text width.
    /// @attention Call this function before addOption() or insertOption() for best results.
    void setEnableAutoAdjustOptionTextWidth(bool enable) { enableAutoAdjustOptionTextWidth_ = enable; }

    /// @brief Set the column separator character. Default is '|'.
    void setColumnSeparator(char separator) { columnSeparator_ = separator; }

    /// @brief Set the row separator character. Default is '-'.
    /// @attention - Use '\0' to disable row separators.
    /// @attention - Row separators are not displayed if option text width is 0.
    void setRowSeparator(char separator) { rowSeparator_ = separator; }

    /// @brief Set the text alignment for option display. Default is 0 (left-aligned).
    /// @note - 0: Left-justified
    /// @note - 1: Right-justified
    /// @note - 2: Center-justified
    /// @attention Alignment has no effect if option text width is 0.
    void setOptionTextAlignment(int alignment) { optionTextAlignment_ = alignment; }

    /// @brief Set the key to confirm/select the highlighted option.
    void setConfirmKey(int key) { confirmKey_ = key; }

    /// @brief Set the key to exit the menu or return to parent menu.
    void setExitKey(int key) { exitKey_ = key; }

    /// @brief Set the directional keys for navigation.
    void setDirectionalControlKey(int left, int up, int right, int down)
    {
        directionalControlKey_ = { left, up, right, down };
    }

    /// @overload
    void setDirectionalControlKey(const std::array<int, 4>& keys) { directionalControlKey_ = keys; }

    /// @brief Set the maximum number of columns for menu layout. Default is 1.
    /// @attention A value of 0 has the same effect as 1.
    void setMaxColumn(size_t maxColumn) { maxColumn_ = maxColumn == 0 ? 1 : maxColumn; }

    /// @brief Set the fixed width for option text display. Default is 0 (auto-width).
    /// @note - If option text exceeds this width, it will be truncated with "...".
    /// @note - If option text is shorter, spaces will be added based on alignment.
    /// @note - A value of 0 disables text justification and row separators.
    void setOptionTextWidth(size_t width) { optionTextWidth_ = width; }

    /// @brief Set the currently highlighted option.
    /// @attention If the index is out of range, the last option will be selected.
    void setHighlightedOption(size_t index)
    {
        if (index >= options_.size())
            setHighlightedOption(options_.size() - 1);
        else
            selectedOption_ = index;
    }

    /// @brief Select the specified option (alias for setHighlightedOption).
    /// @sa setHighlightedOption()
    void selectOption(size_t index) { setHighlightedOption(index); }

    /// @brief Set the background color for option text. Default uses console default.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note Invalid RGB values (e.g., [-1, -1, -1]) restore console default colors.
    void setBackgroundColor(int r, int g, int b) { backgroundColor_ = { r, g, b }; }
#else
    void setBackgroundColor(Rgb color) { backgroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief Set the foreground color for option text. Default uses console default.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note Invalid RGB values (e.g., [-1, -1, -1]) restore console default colors.
    void setForegroundColor(int r, int g, int b) { foregroundColor_ = { r, g, b }; }
#else
    void setForegroundColor(Rgb color) { foregroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief Set the background color for the highlighted option. Default uses console default.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note Invalid RGB values (e.g., [-1, -1, -1]) restore console default colors.
    void setHighlightBackgroundColor(int r, int g, int b) { highlightBackgroundColor_ = { r, g, b }; }
#else
    void setHighlightBackgroundColor(Rgb color) { highlightBackgroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief Set the foreground color for the highlighted option. Default is green.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note Invalid RGB values (e.g., [-1, -1, -1]) restore console default colors.
    void setHighlightForegroundColor(int r, int g, int b) { highlightForegroundColor_ = { r, g, b }; }
#else
    void setHighlightForegroundColor(Rgb color) { highlightForegroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief Set the text to display above the menu.
    void setTopText(const std::string& text) { topText_ = text; }

    /// @brief Set the text to display below the menu.
    void setBottomText(const std::string& text) { bottomText_ = text; }

    /// @brief Select and trigger the specified option.
    /// @attention No exception is thrown even if index is out of range or callback is null.
    void triggerOption(size_t index)
    {
        if (index >= options_.size())
            return;

        selectOption(index);

        if (!options_[index].callback.isValid())
            return;

        if (options_[index].enableNewPage)
            clearConsole();

        options_[selectedOption_].callback.execute();
        if (options_[selectedOption_].waitKeyAfterEnd)
            getkey();

        clearConsole();
    }

    /// @brief Clear the console screen.
    void clearConsole()
    {
    #ifdef _WIN32
        ::system("cls");
    #else
        ::system("clear");
    #endif // _WIN32
    }

    /// @brief Display the menu.
    void show()
    {
        clearConsole();
        update_();
    }

    /// @brief Start receiving keyboard input for menu navigation.
    /// @attention This function blocks the current thread until the input loop is exited.
    void startReceiveInput()
    {
        while (!shouldEndReceiveInput_)
        {
            int key = getkey();
            if (key == confirmKey_)
            {
                triggerOption(selectedOption_);
                update_();
            }
            else if (key == exitKey_)
            {
                shouldEndReceiveInput_ = true;
            }
            // Left
            else if (key == directionalControlKey_[0])
            {
                if (selectedOption_ > 0)
                {
                    selectOption(selectedOption_ - 1);
                    update_();
                }
            }
            // Up
            else if (key == directionalControlKey_[1])
            {
                size_t currentRow = selectedOption_ / maxCol_();
                if (currentRow > 0)
                {
                    selectOption(selectedOption_ - maxCol_());
                    update_();
                }
            }
            // Right
            else if (key == directionalControlKey_[2])
            {
                if (!options_.empty())
                {
                    if (selectedOption_ < options_.size() - 1)
                    {
                        selectOption(selectedOption_ + 1);
                        update_();
                    }
                }
            }
            // Down
            else if (key == directionalControlKey_[3])
            {
                if (!options_.empty())
                {
                    size_t currentRow = selectedOption_ / maxCol_();
                    size_t sumRow = (options_.size() - 1) / maxCol_() + 1;

                    if (currentRow < sumRow - 1)
                    {
                        size_t expectedPos = selectedOption_ + maxCol_();
                        expectedPos = expectedPos < options_.size() ? expectedPos : options_.size() - 1;

                        if (expectedPos != selectedOption_)
                        {
                            selectOption(expectedPos);
                            update_();
                        }
                    }
                }
            }
        }
    }

    /// @brief End the input loop.
    /// @note This function is thread-safe.
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
        bool waitKeyAfterEnd;
        std::string text;
        CallbackFunc callback;
    };

    static std::string cutoffString_(const std::string& str, size_t width)
    {
        if (str.size() <= width)
            return str;
        else
            return str.substr(0, width - 3) + "...";
    }

    static std::string justifyString_(const std::string& str, size_t width, int alignment)
    {
        if (str.size() > width)
            return justifyString_(cutoffString_(str, width), width, alignment);

        switch (alignment)
        {
            case 0:
                return str + std::string(width - str.size(), ' ');
            case 1:
                return std::string(width - str.size(), ' ') + str;
            case 2:
            {
                std::string tmp = std::string((width - str.size()) / 2, ' ') + str;
                tmp += std::string(width - tmp.size(), ' ');
                return tmp;
            }
            default:
                return str;
        }
    }

#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    static bool isVaildColor_(int r, int g, int b)
    {
        return (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255);
    }
#else
    static bool isValidColor(int color)
    {
        return color >= 0 && color <= 255;
    }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // Reset all console attributes.
    static void resetConsoleAttribute_() { std::cout << "\x1b[0m"; }

    // Set the console background color for text.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    static void setConsoleBackgroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
    }
#else
    static void setConsoleBackgroundColor_(Rgb color)
    {
        if (color != COLOR_NONE && isValidColor(color))
            std::cout << "\x1b[48;5;" << color << "m";
    }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // Set the console foreground color for text.
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    static void setConsoleForegroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m";
    }
#else
    static void setConsoleForegroundColor_(Rgb color)
    {
        if (color != COLOR_NONE && isValidColor(color))
            std::cout << "\x1b[38;5;" << color << "m";
    }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // Output text with specified colors.
    static void outputText_(const std::string& text, const Rgb& foregroundColor, const Rgb& backgroundColor)
    {
    #ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
        setConsoleForegroundColor_(foregroundColor[0], foregroundColor[1], foregroundColor[2]);
        setConsoleBackgroundColor_(backgroundColor[0], backgroundColor[1], backgroundColor[2]);
    #else
        setConsoleForegroundColor_(foregroundColor);
        setConsoleBackgroundColor_(backgroundColor);
    #endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

        std::cout << text;

        resetConsoleAttribute_();
    }

    size_t maxCol_() const { return maxColumn_ < getOptionCount() ? maxColumn_ : getOptionCount(); }

    // Update the console display.
    void update_()
    {
        // Clear the console and move the cursor to the top-left corner.
        std::cout << "\x1b[3J\x1b[H";

        // Output the top text if not empty.
        if (!topText_.empty())
            std::cout << topText_ << '\n' << std::endl;

        // Calculate row width based on max columns and option text width.
        // Includes column separators.
        size_t rowWidth = options_.empty() ? 0 : (optionTextWidth_ + 1) * maxCol_() + 1;

        // Output the top row separator if enabled.
        if (rowSeparator_ != '\0' && optionTextWidth_ != 0)
            std::cout << std::string(rowWidth, rowSeparator_) << std::endl;

        for (size_t i = 0; i < options_.size(); ++i)
        {
            std::string text;

            // Add index prefix if enabled.
            if (enableShowIndex_)
                text += "[" + std::to_string(i) + "] ";

            // Append the option text.
            text += options_[i].text;

            // Justify text if optionTextWidth_ is set.
            if (optionTextWidth_ != 0)
                 text = justifyString_(text, optionTextWidth_, optionTextAlignment_);

            std::cout << columnSeparator_;

            // Output option text with appropriate colors.
            if (i == selectedOption_)
                outputText_(text, highlightForegroundColor_, highlightBackgroundColor_);
            else
                outputText_(text, foregroundColor_, backgroundColor_);

            size_t posInRow = i % maxCol_();
            bool isLastOneInRow = posInRow == maxCol_() - 1 || i == options_.size() - 1;
            // Handle end-of-row formatting.
            if (isLastOneInRow)
            {
                // Output the final column separator.
                std::cout << columnSeparator_;

                if (rowSeparator_ == '\0' || optionTextWidth_ == 0)
                {
                    std::cout << std::endl;
                }
                else
                {
                    // Fill missing columns in incomplete rows.
                    if (posInRow != maxCol_() - 1)
                    {
                        size_t supplementWidth = (maxCol_() - posInRow - 1) * (optionTextWidth_ + 1);
                        std::string supplement(supplementWidth, ' ');

                        size_t curpos = optionTextWidth_;
                        for (size_t i = 0; i < maxCol_() - posInRow - 1; ++i)
                        {
                            supplement[curpos] = columnSeparator_;
                            curpos += optionTextWidth_ + 1;
                        }

                        std::cout << supplement;
                    }

                    std::cout << std::endl;

                    // Generate row separator with column separator markers.
                    std::string separator(rowWidth, rowSeparator_);

                    if (i != options_.size() - 1)
                    {
                        size_t curpos = 0;
                        for (size_t i = 0; i < maxCol_() + 1; ++i)
                        {
                            separator[curpos] = columnSeparator_;
                            curpos += optionTextWidth_ + 1;
                        }
                    }

                    std::cout << separator << std::endl;
                }
            }
        }

        // Output the bottom text if not empty.
        if (!bottomText_.empty())
            std::cout << '\n' << bottomText_ << std::endl;

        std::cout << std::endl << std::flush;
    }

    // Reserve space to prevent index text from being truncated during auto-width adjustment.
    static const size_t reserveSpace = 8;

    // Whether to show option indices.
    bool enableShowIndex_                       = false;
    // Whether to auto-adjust option text width based on longest option.
    bool enableAutoAdjustOptionTextWidth_       = true;
    // Column separator character. Default is '|'.
    char columnSeparator_                       = '|';
    // Row separator character. Default is '-'.
    // Use '\0' to disable.
    char rowSeparator_                          = '-';
    // Text alignment for option display. Default is 0 (left-aligned).
    // 0: left-justified, 1: right-justified, 2: center-justified.
    int optionTextAlignment_                    = 0;
    // Key to confirm selection.
#ifdef _WIN32
    int confirmKey_                             = 0x0D;  // Enter key
#else
    int confirmKey_                             = 0x0A;  // Enter key
#endif // _WIN32
    // Key to exit menu.
    int exitKey_                                = 0x1B;  // Escape key
    // Directional control keys: Left, Up, Right, Down.
    std::array<int, 4> directionalControlKey_   = { 'a', 'w', 'd', 's' };
    // Maximum number of columns for layout.
    // Default is 1. Value 0 is treated as 1.
    size_t maxColumn_                           = 1;
    // Fixed width for option text display. Default is 0 (auto-width).
    // 0 disables text justification and row separators.
    size_t optionTextWidth_                     = 0;
    // Currently selected option index.
    size_t selectedOption_                      = 0;
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    Rgb backgroundColor_                        = { -1, -1, -1 };
    Rgb foregroundColor_                        = { -1, -1, -1 };
    Rgb highlightBackgroundColor_               = { -1, -1, -1 };
    Rgb highlightForegroundColor_               = { 0, 255, 0 };
#else
    Rgb backgroundColor_                        = COLOR_NONE;
    Rgb foregroundColor_                        = COLOR_NONE;
    Rgb highlightBackgroundColor_               = COLOR_NONE;
    Rgb highlightForegroundColor_               = COLOR_GREEN;
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR
    std::string topText_;
    std::string bottomText_;
    std::vector<Option> options_;
    // Flag to control input loop termination.
    std::atomic<bool> shouldEndReceiveInput_;
};

#endif // !COMMAND_LINE_MENU_HPP
