// The "Command Line Menu" library, in c++.
//
// Webs: https://github.com/JaderoChan/CommandLineMenu
// You can contact me by email: c_dl_cn@outlook.com

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

    CommandLineMenu() : shouldEndReceiveInput_(false) {};

    ~CommandLineMenu() = default;

    CommandLineMenu(const CommandLineMenu& other) = delete;

    CommandLineMenu& operator=(const CommandLineMenu& other) = delete;

    /// @brief Add a new option to the last position.
    /// @param optionText       The text of the option.
    /// @param callbackFunc     The callback function when the option is triggered.
    /// @param enableNewPage    Whether go to the new page when the option be triggered.
    void addOption(const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, callbackFunc });

        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief Add a new option to the last position.
    /// @param optionText       The text of the option.
    /// @param callbackFunc     The callback function when the option is triggered.
    /// @param arg              The argument of the callback function.
    /// @param enableNewPage    Whether go to the new page when the option be triggered.
    void addOption(const std::string& optionText, ArgFunc callbackFunc, Arg arg, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });

        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @brief Insert a new option to the specified position.
    /// @param index            The position to insert the option.
    /// @param optionText       The text of the option.
    /// @param callbackFunc     The callback function when the option is triggered.
    /// @param enableNewPage    Whether go to the new page when the option be triggered.
    void insertOption(size_t index, const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index, Option { enableNewPage, optionText, callbackFunc });

        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief Insert a new option to the specified position.
    /// @param index            The position to insert the option.
    /// @param optionText       The text of the option.
    /// @param callbackFunc     The callback function when the option is triggered.
    /// @param arg              The argument of the callback function.
    /// @param enableNewPage    Whether go to the new page when the option be triggered.
    void insertOption(size_t index, const std::string& optionText, ArgFunc callbackFunc, Arg arg,
                      bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index,
                        Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });

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

    /// @brief Set the whether go to the new page when the option be triggered.
    void setOptionEnableNewPage(size_t index, bool enable) { options_[index].enableNewPage = enable; }

    /// @brief Set the text of specified option.
    void setOptionText(size_t index, const std::string& text)
    {
        options_[index].text = text;

        if (enableAutoAdjustOptionTextWidth_ && text.size() > optionTextWidth_)
            optionTextWidth_ = text.size();
    }

    /// @brief Set the callback function of specified option.
    void setOptionCallback(size_t index, VoidFunc callbackFunc)
    {
        options_[index].callback = CallbackFunc(callbackFunc);
    }

    /// @overload
    /// @brief Set the callback function and argument of specified option.
    void setOptionCallback(size_t index, ArgFunc callbackFunc, Arg arg)
    {
        options_[index].callback = CallbackFunc(callbackFunc, arg);
    }

    /// @brief Set the argument of specified option.
    /// @attention Only available when the option has argument.
    /// @throw Throw runtime_error exception if the option has no callback function with argument.
    void setOptionCallbackArg(size_t index, Arg arg)
    {
        if (options_[index].callback.isArgFunc)
            options_[index].callback.argFuncArg.second = arg;
        else
            throw std::runtime_error("Specified option has no callback function with argument.");
    }

    /// @brief Get the count of options.
    size_t optionCount() const { return options_.size(); }

    /// @brief Set whether to show the index of each option.
    void setEnableShowIndex(bool enable) { enableShowIndex_ = enable; }

    /// @brief Set whether to show the title (option text) of each option page.
    void setEnableShowOptionPageTitle(bool enable) { enableShowOptionPageTitle_ = enable; }

    /// @brief Set whether to adjust the option text width based on the the longest option text automatically.
    /// @attention This function should be called before addOption() or insertOption().
    void setEnableAutoAdjustOptionTextWidth(bool enable) { enableAutoAdjustOptionTextWidth_ = enable; }

    /// @brief Set the column separator.
    /// Default is '|'.
    void setColumnSeparator(char separator) { columnSeparator_ = separator; }

    /// @brief Set the row separator.
    /// Default is '-', and value '\0' indicating no separator.
    /// @attention If the option text width is 0, the row separator will not be output.
    void setRowSeparator(char separator) { rowSeparator_ = separator; }

    /// @brief Set the alignment of the option text.
    /// Default value is 0.
    /// @note The value 0 indicates that do left justified.
    /// @note The value 1 indicates that do right justified.
    /// @note The value 2 indicates that do center justified.
    /// @attention If the option text width is 0, the alignment is invalid.
    void setOptionTextAlignment(int alignment) { optionTextAlignment_ = alignment; }

    /// @brief Set the enter key, used to trigger the selected option.
    void setEnterKey(int key) { enterKey_ = key; }

    /// @brief Set the esc key, used to return to the main menu or exit the input loop.
    void setEscKey(int key) { escKey_ = key; }

    /// @brief Set the directional control key, used to select option.
    void setDirectionalControlKey(int left, int up, int right, int down)
    {
        directionalControlKey_ = { left, up, right, down };
    }

    /// @overload
    void setDirectionalControlKey(const std::array<int, 4>& keys) { directionalControlKey_ = keys; }

    /// @brief Set the max column of option menu, used to align the output.
    void setMaxColumn(size_t maxColumn) { maxColumn_ = maxColumn == 0 ? 1 : maxColumn; }

    /// @brief Set the justified width of the option text.
    /// Default value is 0.
    /// @note The value 0 indicates that do not justify the text, and the row separator will not be output.
    void setOptionTextWidth(ssize_t width) { optionTextWidth_ = width; }

    /// @brief Set the current selected option (highlight option).
    void setHighlightedOption(size_t index)
    {
        if (index >= options_.size())
            setHighlightedOption(options_.size() - 1);
        else
            selectedOption_ = index;
    }

    /// @brief Select the specified option, the same as setHighlightedOption().
    /// @sa setHighlightOption()
    void selectOption(size_t index) { setHighlightedOption(index); }

    /// @brief Set the background color of the option text.
    void setBackgroundColor(int r, int g, int b) { backgroundColor_ = { r, g, b }; }

    /// @brief Set the foreground color of the option text.
    void setForegroundColor(int r, int g, int b) { foregroundColor_ = { r, g, b }; }

    /// @brief Set the highlight background color of option selected.
    void setHighlightBackgroundColor(int r, int g, int b) { highlightBackgroundColor_ = { r, g, b }; }

    /// @brief Set the highlight foreground color of option selected.
    void setHighlightForegroundColor(int r, int g, int b) { highlightForegroundColor_ = { r, g, b }; }

    /// @brief Set the top text of the option list.
    void setTopText(const std::string& text) { topText_ = text; }

    /// @brief Set the bottom text of the option list.
    void setBottomText(const std::string& text) { bottomText_ = text; }

    /// @brief Set the text be displayed when the option page ended.
    /// Example you can set "Press ESC key to back to the main menu."
    void setNewPageEndedText(const std::string& text) { newPageEndedText_ = text; }

    /// @brief Select and trigger the specified option.
    /// @attention Not throw exception even if the index is out of range or the option's callback function is null.
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
                std::string title;
                if (enableShowIndex_)
                    title = "[" + std::to_string(selectedOption_) + "] ";

                title += options_[selectedOption_].text;

                std::cout << title << std::endl;
                std::cout << std::string(title.size(), '-') << '\n' << std::endl;
            }
        }

        options_[selectedOption_].callback.execute();

        if (options_[index].enableNewPage && !newPageEndedText_.empty())
            std::cout << '\n' << newPageEndedText_ << std::endl;

        if (options_[index].enableNewPage) {
            while (::_getch() != escKey_)
                continue;
        }

        clearConsole();
    }

    /// @brief Clear the console.
    void clearConsole()
    {
#ifdef _WIN32
        ::system("cls");
#else
        ::system("clear");
#endif // _WIN32
    }

    /// @brief Output all options to console.
    void show()
    {
        clearConsole();
        update_();
    }

    /// @brief Start to recving input from console.
    /// @attention This function will block the current thread, and will not return until the input loop is exited.
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
            } else if (key == directionalControlKey_[0]) {  // Left
                if (selectedOption_ > 0) {
                    selectOption(selectedOption_ - 1);
                    update_();
                }
            } else if (key == directionalControlKey_[1]) {  // Up
                size_t currentRow = selectedOption_ / maxColumn_;
                if (currentRow > 0) {
                    selectOption(selectedOption_ - maxColumn_);
                    update_();
                }
            } else if (key == directionalControlKey_[2]) {  // Right
                if (!options_.empty()) {
                    if (selectedOption_ < options_.size() - 1) {
                        selectOption(selectedOption_ + 1);
                        update_();
                    }
                }
            } else if (key == directionalControlKey_[3]) {  // Down
                if (!options_.empty()) {
                    size_t currentRow = selectedOption_ / maxColumn_;
                    size_t sumRow = (options_.size() - 1) / maxColumn_ + 1;

                    if (currentRow < sumRow - 1) {
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
        std::string text;
        CallbackFunc callback;
    };

    static std::string cutoffString(const std::string& str, size_t width)
    {
        if (str.size() <= width)
            return str;
        else
            return str.substr(0, width - 3) + "...";
    }

    static std::string justifyString(const std::string& str, size_t width, int alignment)
    {
        if (str.size() > width)
            return justifyString(cutoffString(str, width), width, alignment);

        switch (alignment) {
            case 0:
                return str + std::string(width - str.size(), ' ');
            case 1:
                return std::string(width - str.size(), ' ') + str;
            case 2: {
                std::string tmp = std::string((width - str.size()) / 2, ' ') + str;
                tmp += std::string(width - tmp.size(), ' ');
                return tmp;
            }
            default:
                return str;
        }
    }

    bool isVaildColor_(int r, int g, int b) const
    {
        return (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255);
    }

    // Reset all console attributes.
    void resetConsoleAttribute_() { std::cout << "\033[0m"; }

    // Set the console background color of text.
    void setConsoleBackgroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";
    }

    // Set the console foreground color of text.
    void setConsoleForegroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    }

    // Output the text with specified color.
    void outputText_(const std::string& text, const Rgb& foregroundColor, const Rgb& backgroundColor)
    {
        setConsoleForegroundColor_(foregroundColor[0], foregroundColor[1], foregroundColor[2]);
        setConsoleBackgroundColor_(backgroundColor[0], backgroundColor[1], backgroundColor[2]);

        std::cout << text;

        resetConsoleAttribute_();
    }

    // Update the console output.
    void update_()
    {
        // Clear the console and move the cursor to the top left position.
        std::cout << "\033[3J\033[H";

        // Output the top text if #topText_ is not empty.
        if (!topText_.empty())
            std::cout << topText_ << '\n' << std::endl;

        // Calculate the width of each row, based on the max column and option text width.
        // And attention, that conatins all column separator.
        size_t rowWidth = options_.empty() ? 0 : (optionTextWidth_ + 1) * maxColumn_ + 1;

        // Output the row separator at top first, if #rowSeparator_ is not '\0'.
        if (rowSeparator_ != '\0' && optionTextWidth_ != 0)
            std::cout << std::string(rowWidth, rowSeparator_) << std::endl;

        for (size_t i = 0; i < options_.size(); ++i) {
            std::string text;

            // Get the index text of option if #enableShowIndex_ is true.
            if (enableShowIndex_)
                text += "[" + std::to_string(i) + "] ";

            // Get the full option text (with index if #enableShowIndex_ is true).
            text += options_[i].text;

            // Adjust the option text width and justify the text if the #optionTextWidth_ is not 0.
            if (optionTextWidth_ != 0)
                 text = justifyString(text, optionTextWidth_, optionTextAlignment_);

            std::cout << columnSeparator_;

            // Output the full option text with specified color.
            if (i == selectedOption_) {
                outputText_(text, highlightForegroundColor_, highlightBackgroundColor_);
            } else {
                outputText_(text, foregroundColor_, backgroundColor_);
            }

            size_t posInRow = i % maxColumn_;
            bool isLastOneInRow = posInRow == maxColumn_ - 1 || i == options_.size() - 1;
            // If current option is the last one in the row output the row separator.
            if (isLastOneInRow) {
                // First, output the column separator for the last one in the row or the last one in the option list.
                std::cout << columnSeparator_;

                if (rowSeparator_ == '\0' || optionTextWidth_ == 0) {
                    std::cout << std::endl;
                } else {
                    if (posInRow != maxColumn_ - 1) {
                        size_t supplementWidth = (maxColumn_ - posInRow - 1) * (optionTextWidth_ + 1);
                        std::string supplement(supplementWidth, ' ');

                        size_t curpos = optionTextWidth_;
                        for (size_t i = 0; i < maxColumn_ - posInRow - 1; ++i) {
                            supplement[curpos] = columnSeparator_;
                            curpos += optionTextWidth_ + 1;
                        }

                        std::cout << supplement;
                    }

                    std::cout << std::endl;

                    std::string separator(rowWidth, rowSeparator_);

                    if (i != options_.size() - 1) {
                        size_t curpos = 0;
                        for (size_t i = 0; i < maxColumn_ + 1; ++i) {
                            separator[curpos] = columnSeparator_;
                            curpos += optionTextWidth_ + 1;
                        }
                    }

                    std::cout << separator << std::endl;
                }
            }
        }

        // Output the bottom text if #bottomText_ is not empty.
        if (!bottomText_.empty())
            std::cout << '\n' << bottomText_ << std::endl;

        std::cout << std::endl << std::flush;
    }

    // The value of reserve space to prevent the index of option out of range
    // when adjust the option text width automatically.
    static const size_t reserveSpace = 8;

    // Whether to show the index of each option.
    bool enableShowIndex_                       = false;
    // Whether to show the title (option text) at top of option page.
    bool enableShowOptionPageTitle_             = false;
    // Whether to adjust the option text width based on the the longest option text automatically.
    bool enableAutoAdjustOptionTextWidth_       = false;
    // Separator of each column. (default is '|')
    char columnSeparator_                       = '|';
    // Separator of each row. (default is '-', and value '\0' indicating no separator)
    char rowSeparator_                          = '-';
    // The alignment of option text, used to align the output.
    // Default value is 0.
    // The value 0 indicates that do left justified.
    // The value 1 indicates that do right justified.
    // The value 2 indicates that do center justified.
    // (based on the current option text length and the OptionTextWidth)
    int optionTextAlignment_                    = 0;
    // Enter key, used to trigger the option.
    int enterKey_                               = 0x0D;
    // Esc key, used to return to the main menu or exit the input loop.
    int escKey_                                 = 0x1B;
    // Directional control key, used to select option.
    // Left, Up, Right, Down
    std::array<int, 4> directionalControlKey_   = { 'a', 'w', 'd', 's' };
    // The max column of option list, used to align the output.
    // Default value is 1, and value 0 is same to value 1.
    size_t maxColumn_                           = 1;
    // The justified width of option text, used to align the output.
    // Default value is 0.
    // The value 0 indicates that do not justify the text, and the row separator will not be output.
    size_t optionTextWidth_                     = 0;
    // Current selected option index.
    size_t selectedOption_                      = 0;
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
