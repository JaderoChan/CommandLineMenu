#ifndef COMMAND_LINE_OPTION_HPP
#define COMMAND_LINE_OPTION_HPP

#include <cstddef>      // size_t
#include <conio.h>      // _getch()
#include <string>       // string
#include <array>        // array
#include <vector>       // vector
#include <iostream>     // cout, endl, flush, getchar()
#include <stdexcept>    // runtime_error

class CommandLineOption
{
public:
    using Rgb           = std::array<int, 3>;
    using VoidFunc      = void (*)();
    using Arg           = void*;
    using ArgFunc       = void (*)(Arg);

    static CommandLineOption& getInstance()
    {
        static CommandLineOption instance;
        return instance;
    }

    // @brief Add a new option to back.
    // @param optionText The text of the option.
    // @param callbackFunc The callback function when the option is triggered.
    void addOption(const std::string& optionText, VoidFunc callbackFunc, bool immediateUpdate = false)
    {
        optionTexts_.push_back(optionText);
        callbackFuncs_.push_back(CallbackFunc(callbackFunc));

        if (immediateUpdate)
            update_();
    }

    // @brief Add a new option with argument to back.
    // @param optionText The text of the option.
    // @param callbackFunc The callback function when the option is triggered.
    // @param arg The argument of the callback function.
    void addOption(const std::string& optionText, ArgFunc callbackFunc, Arg arg, bool immediateUpdate = false)
    {
        optionTexts_.push_back(optionText);
        callbackFuncs_.push_back(CallbackFunc(callbackFunc, arg));

        if (immediateUpdate)
            update_();
    }

    // @brief Insert a new option to the specified position.
    // @param index The position to insert the option.
    // @param optionText The text of the option.
    // @param callbackFunc The callback function when the option is triggered.
    void insertOption(size_t index, const std::string& optionText, VoidFunc callbackFunc, bool immediateUpdate = false)
    {
        optionTexts_.insert(optionTexts_.begin() + index, optionText);
        callbackFuncs_.insert(callbackFuncs_.begin() + index, CallbackFunc(callbackFunc));

        if (immediateUpdate)
            update_();
    }

    // @brief Insert a new option with argument to the specified position.
    // @param index The position to insert the option.
    // @param optionText The text of the option.
    // @param callbackFunc The callback function when the option is triggered.
    // @param arg The argument of the callback function.

    void insertOption(size_t index, const std::string& optionText, ArgFunc callbackFunc, Arg arg,
                      bool immediateUpdate = false)
    {
        optionTexts_.insert(optionTexts_.begin() + index, optionText);
        callbackFuncs_.insert(callbackFuncs_.begin() + index, CallbackFunc(callbackFunc, arg));

        if (immediateUpdate)
            update_();
    }

    // @brief Set the text of specified option.
    // @param index The index of the option.
    void setOptionText(size_t index, const std::string& optionText, bool immediateUpdate = false)
    {
        optionTexts_[index] = optionText;

        if (immediateUpdate)
            update_();
    }

    // @brief Set the callback function of specified option.
    // @param index The index of the option.
    void setOptionCallback(size_t index, VoidFunc callbackFunc)
    {
        callbackFuncs_[index] = CallbackFunc(callbackFunc);
    }

    // @brief Set the callback function and argument of specified option.
    // @param index The index of the option.
    void setOptionCallback(size_t index, ArgFunc callbackFunc, Arg arg)
    {
        callbackFuncs_[index] = CallbackFunc(callbackFunc, arg);
    }

    // @brief Set the argument of specified option.
    // @attention Only available when the option has argument.
    // @param index The index of the option.
    // @throw runtime_error If the option has no callback function with argument.
    void setOptionArg(size_t index, Arg arg)
    {
        if (callbackFuncs_[index].isArgFunc)
            callbackFuncs_[index].argFuncArg.second = arg;
        else
            throw std::runtime_error("Specified option has no callback function with argument.");
    }

    // @brief Remove an option by its index.
    void removeOption(size_t index, bool immediateUpdate = false)
    {
        optionTexts_.erase(optionTexts_.begin() + index);
        callbackFuncs_.erase(callbackFuncs_.begin() + index);

        if (immediateUpdate)
            update_();
    }

    // @brief Remove all options.
    void removeAllOption(bool immediateUpdate = false)
    {
        optionTexts_.clear();
        callbackFuncs_.clear();

        if (immediateUpdate)
            update_();
    }

    // @brief Get the count of options.
    size_t optionCount() const { return optionTexts_.size(); }

    // @brief Set whether to show the index of each option.
    void setShowIndex(bool show, bool immediateUpdate = false)
    {
        isShowIndex_ = show;

        if (immediateUpdate)
            update_();
    }

    // @brief Set the max column of option list, used to align the output.
    void setMaxColumn(size_t maxColumn, bool immediateUpdate = false)
    {
        maxColumn_ = maxColumn == 0 ? 1 : maxColumn;

        if (immediateUpdate)
            update_();
    }

    // @brief Set the enter key, used to trigger the selected option.
    void setEnterKey(int key) { enterKey_ = key; }

    // @brief Set the esc key, used to exit the input loop.
    void setEscKey(int key) { escKey_ = key; }

    // @brief Set the directional control key, used to select option.
    void setDirectionalControlKey(int left, int up, int right, int down)
    {
        directionalControlKey_ = { left, up, right, down };
    }

    // @brief Set the background color of the option text.
    void setBackgroundColor(int r, int g, int b, bool immediateUpdate = false)
    {
        backgroundColor_ = { r, g, b };

        if (immediateUpdate)
            update_();
    }

    // @brief Set the foreground color of the option text.
    void setForegroundColor(int r, int g, int b, bool immediateUpdate = false)
    {
        foregroundColor_ = { r, g, b };

        if (immediateUpdate)
            update_();
    }

    // @brief Set the highlight background color of option selected.
    void setHighlightBackgroundColor(int r, int g, int b, bool immediateUpdate = false)
    {
        highlightBackgroundColor_ = { r, g, b };

        if (immediateUpdate)
            update_();
    }

    // @brief Set the highlight foreground color of option selected.
    void setHighlightForegroundColor(int r, int g, int b, bool immediateUpdate = false)
    {
        highlightForegroundColor_ = { r, g, b };

        if (immediateUpdate)
            update_();
    }

    // @brief Set the current selected option (highlighted option).
    void setHighlightedOption(size_t index, bool immediateUpdate = false)
    {
        selectedOption_ = index;

        if (immediateUpdate)
            update_();
    }

    // @brief Set the top text of the option list.
    void setTopText(const std::string& topText, bool immediateUpdate = false)
    {
        topText_ = topText;

        if (immediateUpdate)
            update_();
    }

    // @brief Set the bottom text of the option list.
    void setBottomText(const std::string& bottomText, bool immediateUpdate = false)
    {
        bottomText_ = bottomText;

        if (immediateUpdate)
            update_();
    }

    // @brief Select the specified option, same of setHighlightedOption().
    void selectOption(size_t index, bool immediateUpdate = false) { setHighlightedOption(index, immediateUpdate); }

    // @brief Select and trigger the specified option.
    // @note Not throw exception even if the index is out of range or the option's callback function is null.
    void triggerOption(size_t index, bool immediateUpdate = false)
    {
        if (index >= optionTexts_.size())
            return;

        selectOption(index, immediateUpdate);

        if (callbackFuncs_[index].isArgFunc) {
            if (callbackFuncs_[index].argFuncArg.first)
                callbackFuncs_[index].argFuncArg.first(callbackFuncs_[index].argFuncArg.second);
        } else {
            if (callbackFuncs_[index].voidFunc)
                callbackFuncs_[index].voidFunc();
        }
    }

    // @brief Clear the console.
    void clearConsole() { std::cout << "\x1b[2J\x1b[H"; }

    // @brief Output all options to console.
    void show()
    {
        update_();
    }

    // @brief Start to recving input from console.
    // @note This function will block the current thread, and will not return until the input loop is exited.
    void startRecvingInput()
    {
        bool shouldEndRecvingInput = false;

        while (!shouldEndRecvingInput) {
            // Clear the input buffer.
            std::cin.clear();

            int key = ::_getch();

            if (key == enterKey_) {
                triggerOption(selectedOption_);
            } else if (key == escKey_) {
                shouldEndRecvingInput = true;
            } else if (key == directionalControlKey_[0]) {
                if (selectedOption_ > 0)
                    selectOption(selectedOption_ - 1, true);
            } else if (key == directionalControlKey_[1]) {
                size_t currentRow = selectedOption_ / maxColumn_;
                if (currentRow > 0)
                    selectOption(selectedOption_ - maxColumn_, true);
            } else if (key == directionalControlKey_[2]) {
                if (selectedOption_ < optionTexts_.size() - 1)
                    selectOption(selectedOption_ + 1, true);
            } else if (key == directionalControlKey_[3]) {
                size_t currentRow = selectedOption_ / maxColumn_;
                if (currentRow < optionTexts_.size() / maxColumn_) {
                    size_t expectedPos = selectedOption_ + maxColumn_;
                    expectedPos = expectedPos < optionTexts_.size() ? expectedPos : optionTexts_.size() - 1;
                    if (expectedPos != selectedOption_)
                        selectOption(expectedPos, true);
                }
            }
        }
    }

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

        bool isArgFunc;
        union
        {
            VoidFunc voidFunc;
            std::pair<ArgFunc, Arg> argFuncArg;
        };
    };

    CommandLineOption() = default;

    CommandLineOption(const CommandLineOption&) = delete;

    CommandLineOption& operator=(const CommandLineOption&) = delete;

    bool isVaildColor_(int r, int g, int b) const
    {
        return (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255);
    }

    // @brief Reset all console attributes.
    void resetConsoleColor_() { std::cout << "\x1b[0m"; }

    // @brief Set the console background color of text.
    void setConsoleBackgroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
    }

    // @brief Set the console foreground color of text.
    void setConsoleForegroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m";
    }

    // @brief Output the text with specified color.
    // @param text The text to output.
    // @param foregroundColor The foreground color of text.
    // @param backgroundColor The background color of text.
    void outputText_(const std::string& text, const Rgb& foregroundColor, const Rgb& backgroundColor)
    {
        setConsoleForegroundColor_(foregroundColor[0], foregroundColor[1], foregroundColor[2]);
        setConsoleBackgroundColor_(backgroundColor[0], backgroundColor[1], backgroundColor[2]);

        std::cout << text;

        resetConsoleColor_();
    }

    // @brief Update the console output.
    void update_()
    {
        clearConsole();

        if (!topText_.empty())
            std::cout << topText_ << '\n' << std::endl;

        for (size_t i = 0; i < optionTexts_.size(); ++i) {
            if (isShowIndex_)
                std::cout << "[" << i << "] ";

            if (i == selectedOption_) {
                outputText_(optionTexts_[i], highlightForegroundColor_, highlightBackgroundColor_);
            } else {
                outputText_(optionTexts_[i], foregroundColor_, backgroundColor_);
            }

            if (maxColumn_ == 0 || i % maxColumn_ == maxColumn_ - 1 || i == optionTexts_.size() - 1)
                std::cout << std::endl;
            else
                std::cout << '\t';
        }

        if (!bottomText_.empty())
            std::cout << '\n' << bottomText_ << std::endl;

        std::cout << std::endl << std::flush;
    }

    // Whether to show the index of each option.
    bool isShowIndex_                           = false;
    // The max column of option list, used to align the output.
    // Default value is 1, and value 0 is same to value 1.
    size_t maxColumn_                           = 1;
    // Current selected option index.
    size_t selectedOption_                      = 0;
    // Enter key, used to trigger the selected option.
    int enterKey_                               = 0x0D;
    // Esc key, used to exit the input loop.
    int escKey_                                 = 0x1B;
    // Directional control key, used to select option.
    // Left, Up, Right, Down
    std::array<int, 4> directionalControlKey_   = { 'a', 'w', 'd', 's' };
    // Default colir is invaid, indicating that do not set color.
    Rgb backgroundColor_                        = { -1, -1, -1 };
    Rgb foregroundColor_                        = { 255, 255, 255 };
    Rgb highlightBackgroundColor_               = { 255, 255, 0 };
    Rgb highlightForegroundColor_               = { 0, 0, 255 };
    std::string topText_;
    std::string bottomText_;
    std::vector<std::string> optionTexts_;
    std::vector<CallbackFunc> callbackFuncs_;
};

#endif // !COMMAND_LINE_OPTION_HPP
