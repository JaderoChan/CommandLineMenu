// Command Line Menu 库
//
// 网址链接：https://github.com/JaderoChan/CommandLineMenu
// 你可以通过邮箱联系我：c_dl_cn@outlook.com

// MIT License
//
// Copyright (c) 2024 頔珞JaderoChan
//
// 特此免费授予任何人获得本软件和相关文档文件（“软件”）的副本，不受限制地处理本软件，
// 包括但不限于使用、复制、修改、合并、发布、分发、再许可和/或销售本软件副本的权利，
// 并允许提供本软件的人这样做，但须符合以下条件：
//
// 上述版权声明和本许可声明应包含在本软件的所有副本或主要部分中。
//
// 本软件“按原样”提供，不提供任何形式的明示或暗示的保证，包括但不限于适销性、适用于特定目的和不侵权的保证。
// 在任何情况下，作者或版权所有人都不承担任何索赔、损害赔偿或其他责任，无论是在合同诉讼、侵权行为或其他诉讼中，
// 由软件、软件的使用或其他交易引起的或与之有关的。

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
    #include <conio.h>      // _getch()
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
    enum Rgb
    {
        RGB_NONE    = 0,
        RGB_BLACK   = 30,
        RGB_RED     = 31,
        RGB_GREEN   = 32,
        RGB_YELLOW  = 33,
        RGB_BLUE    = 34,
        RGB_MAGENTA = 35,
        RGB_CYAN    = 36,
        RGB_WHITE   = 37
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

    /// @brief 在尾部添加一个新选项。
    /// @param optionText       选项文本。
    /// @param callbackFunc     选项被触发时的回调函数。
    /// @param enableNewPage    选项被触发时是否跳转到新页面。
    void addOption(const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, callbackFunc });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief 在尾部添加一个新选项。
    /// @param optionText       选项文本。
    /// @param callbackFunc     选项被触发时的回调函数。
    /// @param arg              回调函数的参数。
    /// @param enableNewPage    选项被触发时是否跳转到新页面。
    void addOption(const std::string& optionText, ArgFunc callbackFunc, Arg arg, bool enableNewPage = true)
    {
        options_.push_back(Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @brief 在指定位置插入新选项。
    /// @param index            插入位置。
    /// @param optionText       选项文本。
    /// @param callbackFunc     选项被触发时的回调函数。
    /// @param enableNewPage    选项被触发时是否跳转到新页面。
    void insertOption(size_t index, const std::string& optionText, VoidFunc callbackFunc, bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index, Option { enableNewPage, optionText, callbackFunc });
        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @overload
    /// @brief 在指定位置插入新选项。
    /// @param index            插入位置。
    /// @param optionText       选项文本。
    /// @param callbackFunc     选项被触发时的回调函数。
    /// @param arg              回调函数的参数。
    /// @param enableNewPage    选项被触发时是否跳转到新页面。
    void insertOption(size_t index, const std::string& optionText, ArgFunc callbackFunc, Arg arg,
                      bool enableNewPage = true)
    {
        options_.insert(options_.begin() + index,
                        Option { enableNewPage, optionText, CallbackFunc(callbackFunc, arg) });

        if (enableAutoAdjustOptionTextWidth_ && optionText.size() + reserveSpace > optionTextWidth_)
            optionTextWidth_ = optionText.size() + reserveSpace;
    }

    /// @brief 移除指定选项。
    void removeOption(size_t index) { options_.erase(options_.begin() + index); }

    /// @brief 移除所有选项。
    void removeAllOption()
    {
        options_.clear();
        if (enableAutoAdjustOptionTextWidth_)
            optionTextWidth_ = 0;
    }

    /// @brief 设置指定选项被触发时是否跳转至新页面。
    void setOptionEnableNewPage(size_t index, bool enable) { options_[index].enableNewPage = enable; }

    /// @brief 设置指定选项的文本。
    void setOptionText(size_t index, const std::string& text)
    {
        options_[index].text = text;
        if (enableAutoAdjustOptionTextWidth_ && text.size() > optionTextWidth_)
            optionTextWidth_ = text.size();
    }

    /// @brief 设置指定选项触发时的回调函数。
    void setOptionCallback(size_t index, VoidFunc callbackFunc)
    {
        options_[index].callback = CallbackFunc(callbackFunc);
    }

    /// @overload
    /// @brief 设置指定选项触发时的带参回调函数。
    void setOptionCallback(size_t index, ArgFunc callbackFunc, Arg arg)
    {
        options_[index].callback = CallbackFunc(callbackFunc, arg);
    }

    /// @brief 设置指定选项带参回调函数的参数。
    /// @attention 只在选项的回调函数为带参函数时有效。
    /// @throw 如果指定选项的回调函数不是带参函数，抛出 std::runtime_error 异常。
    void setOptionCallbackArg(size_t index, Arg arg)
    {
        if (options_[index].callback.isArgFunc)
            options_[index].callback.argFuncArg.second = arg;
        else
            throw std::runtime_error("Specified option has no callback function with argument.");
    }

    /// @brief 获取选项总数。
    size_t optionCount() const { return options_.size(); }

    /// @brief 设置是否为每个选项显示其索引值。
    void setEnableShowIndex(bool enable) { enableShowIndex_ = enable; }

    /// @brief 设置是否基于最长的选项文本自动调整选项文本宽度。
    /// @attention 这个函数应该在 addOption() 和 insertOption() 之前调用。
    void setEnableAutoAdjustOptionTextWidth(bool enable) { enableAutoAdjustOptionTextWidth_ = enable; }

    /// @brief 设置列分隔符。默认为：|。
    void setColumnSeparator(char separator) { columnSeparator_ = separator; }

    /// @brief 设置行分隔符。默认为：-。
    /// @attention - 如果其等于 \0 则不会显示任何行分隔符。
    /// @attention - 如果选项文本宽度设置为0（此时选项文本将不被对齐，紧凑排布），则行分隔符无效（等同于其值为\0时情况）。
    void setRowSeparator(char separator) { rowSeparator_ = separator; }

    /// @brief 设置选项文本的布局方式。默认为0。
    /// @note - 0代表左对齐。
    /// @note - 1代表右对齐。
    /// @note - 2代表居中对齐。
    /// @attention 如果选项文本宽度设置为0，则布局方式无效。
    void setOptionTextAlignment(int alignment) { optionTextAlignment_ = alignment; }

    /// @brief 设置确认键，其用于触发选项。
    void setConfirmKey(int key) { confirmKey_ = key; }

    /// @brief 设置退出键，其用于从选项页面返回菜单页面，以及用于退出输入循环。
    void setExitKey(int key) { exitKey_ = key; }

    /// @brief 设置方向导航键。
    void setDirectionalControlKey(int left, int up, int right, int down)
    {
        directionalControlKey_ = { left, up, right, down };
    }

    /// @overload
    void setDirectionalControlKey(const std::array<int, 4>& keys) { directionalControlKey_ = keys; }

    /// @brief 设置菜单最大列数，用于布局菜单选项。默认为1。
    /// @attention 设置0与设置1的效果效果相同。
    void setMaxColumn(size_t maxColumn) { maxColumn_ = maxColumn == 0 ? 1 : maxColumn; }

    /// @brief 设置选项的文本宽度，用于布局菜单选项。默认为0。
    /// @note - 如果选项文本大于此值则会将溢出部分更改为 ... 以示省略，否则将按照对齐方式将文本前后添置空格。
    /// @note - 如果为0则表示不对文本进行对齐与布局，并且行分隔符将被禁用。
    void setOptionTextWidth(size_t width) { optionTextWidth_ = width; }

    /// @brief 设置当前选中（高光）的项。
    /// @attention 如果所给索引超出范围，则将选中菜单的最后一个选项。
    void setHighlightedOption(size_t index)
    {
        if (index >= options_.size())
            setHighlightedOption(options_.size() - 1);
        else
            selectedOption_ = index;
    }

    /// @brief 选中指定项，与 setHighlightedOption() 相同。
    /// @sa setHighlightOption()
    void selectOption(size_t index) { setHighlightedOption(index); }

    /// @brief 设置非高光选项的文本背景颜色。默认由控制台决定。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note 当r，g，b值无效时（例如[-1, -1, -1])，将会恢复为控制台默认的颜色。
    void setBackgroundColor(int r, int g, int b) { backgroundColor_ = { r, g, b }; }
#else
    void setBackgroundColor(Rgb color) { backgroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief 设置非高光选项的文本前景颜色。默认由控制台决定。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note 当r，g，b值无效时（例如[-1, -1, -1])，将会恢复为控制台默认的颜色。
    void setForegroundColor(int r, int g, int b) { foregroundColor_ = { r, g, b }; }
#else
    void setForegroundColor(Rgb color) { foregroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief 设置高光选项的文本背景颜色。默认由控制台决定。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
/// @note 当r，g，b值无效时（例如[-1, -1, -1])，将会恢复为控制台默认的颜色。
void setHighlightBackgroundColor(int r, int g, int b) { highlightBackgroundColor_ = { r, g, b }; }
#else
void setHighlightBackgroundColor(Rgb color) { highlightBackgroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

/// @brief 设置高光选项的文本前景颜色。默认为绿色，rgb[0, 255, 0]。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    /// @note 当r，g，b值无效时（例如[-1, -1, -1])，将会恢复为控制台默认的颜色。
    void setHighlightForegroundColor(int r, int g, int b) { highlightForegroundColor_ = { r, g, b }; }
#else
    void setHighlightForegroundColor(Rgb color) { highlightForegroundColor_ = color; }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    /// @brief 设置菜单顶部的文本。
    void setTopText(const std::string& text) { topText_ = text; }

    /// @brief 设置菜单底部的文本。
    /// 例如你可以设置："按WASD选择选项，按Enter确认，按Esc退出。"
    void setBottomText(const std::string& text) { bottomText_ = text; }

    /// @brief 选中并触发指定选项。
    /// @attention 即使所给索引值超出范围或者选项的回调函数为空，也不会抛出异常。
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

        clearConsole();
    }

    /// @brief 清空控制台。
    void clearConsole()
    {
    #ifdef _WIN32
        ::system("cls");
    #else
        ::system("clear");
    #endif // _WIN32
    }

    /// @brief 显示菜单。
    void show()
    {
        clearConsole();
        update_();
    }

    /// @brief 开始接收输入循环。
    /// @attention 这个函数将会阻塞主线程，直到按下 Esc 键，或者调用 endReceiveInput() 函数。
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
            // 左
            else if (key == directionalControlKey_[0])
            {
                if (selectedOption_ > 0)
                {
                    selectOption(selectedOption_ - 1);
                    update_();
                }
            }
            // 上
            else if (key == directionalControlKey_[1])
            {
                size_t currentRow = selectedOption_ / maxCol_();
                if (currentRow > 0)
                {
                    selectOption(selectedOption_ - maxCol_());
                    update_();
                }
            }
            // 右
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
            // 下
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

    /// @brief 结束输入循环。
    /// @note 此函数是线程安全的。
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
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // 重置所有控制台属性。
    static void resetConsoleAttribute_() { std::cout << "\x1b[0m"; }

    // 设置控制台的文本背景色。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    static void setConsoleBackgroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[48;2;" << r << ";" << g << ";" << b << "m";
    }
#else
    static void setConsoleBackgroundColor_(Rgb color)
    {
        if (color != RGB_NONE)
            std::cout << "\x1b[48;5;" << color << "m";
    }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // 设置控制台的文本前景色。
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    static void setConsoleForegroundColor_(int r, int g, int b)
    {
        if (isVaildColor_(r, g, b))
            std::cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m";
    }
#else
    static void setConsoleForegroundColor_(Rgb color)
    {
        if (color != RGB_NONE)
            std::cout << "\x1b[38;5;" << color << "m";
    }
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR

    // 以指定颜色输出文本至控制台。
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

    size_t maxCol_() const { return maxColumn_ < optionCount() ? maxColumn_ : optionCount(); }

    // 更新控制台显示。
    void update_()
    {
        // 清空控制台，并将光标移至左上角。
        std::cout << "\x1b[3J\x1b[H";

        // 如果 topText_ 不为空，则输出至控制台。
        if (!topText_.empty())
            std::cout << topText_ << '\n' << std::endl;

        // 基于 maxColumn_ 和 optionTextWidth_，计算一行的字符总长度（包含所有列分隔符在内），
        // 如果 optionTextWidth_ 为0，则此值无效。
        size_t rowWidth = options_.empty() ? 0 : (optionTextWidth_ + 1) * maxCol_() + 1;

        // 如果行分隔符不为\0，则首先输出一行行分隔符（界面顶部的边框）。
        if (rowSeparator_ != '\0' && optionTextWidth_ != 0)
            std::cout << std::string(rowWidth, rowSeparator_) << std::endl;

        // 遍历所有选项。
        for (size_t i = 0; i < options_.size(); ++i)
        {
            std::string text;

            // 如果启用了显示选项索引，则获取选项对应的索引文本。
            if (enableShowIndex_)
                text += "[" + std::to_string(i) + "] ";

            // 获取完整的选项文本（选项文本与可能的索引文本）。
            text += options_[i].text;

            // 如果选项文本宽度不为0，则根据布局方式调整选项文本。
            if (optionTextWidth_ != 0)
                 text = justifyString(text, optionTextWidth_, optionTextAlignment_);

            std::cout << columnSeparator_;

            // 以选项状态对应的颜色输出完整选项文本。
            if (i == selectedOption_)
                outputText_(text, highlightForegroundColor_, highlightBackgroundColor_);
            else
                outputText_(text, foregroundColor_, backgroundColor_);

            size_t posInRow = i % maxCol_();
            bool isLastOneInRow = posInRow == maxCol_() - 1 || i == options_.size() - 1;
            // 如果当前选项位于行尾，则输出一行行分隔符。
            if (isLastOneInRow)
            {
                // 首先输出一个列分隔符作为行的结束（菜单的右边框）。
                std::cout << columnSeparator_;

                // 如果行分隔符为\0或者选项文本宽度为0，则不输出行分隔符。
                if (rowSeparator_ == '\0' || optionTextWidth_ == 0)
                {
                    std::cout << std::endl;
                }
                else
                {
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

        // 如果 bottomText_ 不为空，则输出至控制台。
        if (!bottomText_.empty())
            std::cout << '\n' << bottomText_ << std::endl;

        std::cout << std::endl << std::flush;
    }

    // 空间保留值，用来防止在自动调整选项文本宽度时，由于索引的显示与否造成的可能溢出。
    static const size_t reserveSpace = 8;

    // 控制是否显示选项的索引。
    bool enableShowIndex_                       = false;
    // 控制是否基于最长的选项文本自动调整选项文本宽度。
    bool enableAutoAdjustOptionTextWidth_       = true;
    // 列分隔符。默认为：|。
    char columnSeparator_                       = '|';
    // 行分隔符。默认为：-。
    // 如果其等于 \0 则不会显示任何行分隔符。
    char rowSeparator_                          = '-';
    // 设置选项文本的布局方式。默认为0。
    // 0代表左对齐。
    // 1代表右对齐。
    // 2代表居中对齐。
    int optionTextAlignment_                    = 0;
    // 确认键，其用于触发选项。
#ifdef _WIN32
    int confirmKey_                             = 0x0D;
#else
    int confirmKey_                             = 0x0A;
#endif // _WIN32
    // 退出键，其用于从选项页面返回菜单页面，以及用于退出输入循环。
    int exitKey_                                 = 0x1B;
    // 方向导航键，左上右下。
    std::array<int, 4> directionalControlKey_   = { 'a', 'w', 'd', 's' };
    // 菜单最大列数，用于布局菜单选项。默认为1。
    // 不可为0，利用函数 setmaxCol_() 中设置0时，实际将会设置为1。
    size_t maxColumn_                           = 1;
    // 选项的文本宽度，用于布局菜单选项。默认为0。
    // 如果为0则表示不对文本进行对齐与布局，并且行分隔符将被禁用。
    size_t optionTextWidth_                     = 0;
    // 当前选中（高光）的项。
    size_t selectedOption_                      = 0;
#ifdef COMMAND_LINE_MENU_USE_24BIT_COLOR
    Rgb backgroundColor_                        = { -1, -1, -1 };
    Rgb foregroundColor_                        = { -1, -1, -1 };
    Rgb highlightBackgroundColor_               = { -1, -1, -1 };
    Rgb highlightForegroundColor_               = { 0, 255, 0 };
#else
    Rgb backgroundColor_                        = RGB_NONE;
    Rgb foregroundColor_                        = RGB_NONE;
    Rgb highlightBackgroundColor_               = RGB_NONE;
    Rgb highlightForegroundColor_               = RGB_GREEN;
#endif // COMMAND_LINE_MENU_USE_24BIT_COLOR
    std::string topText_;
    std::string bottomText_;
    std::vector<Option> options_;
    // 控制是否结束输入循环。
    std::atomic<bool> shouldEndReceiveInput_;
};

#endif // !COMMAND_LINE_MENU_HPP
