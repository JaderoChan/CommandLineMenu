#include <command_line_option.hpp>

int main()
{
    CommandLineOption& option = CommandLineOption::getInstance();

    option.setMaxColumn(3);
    option.setShowIndex(true);
    option.setHighlightForegroundColor(0x00, 0x99, 0x33);
    option.setHighlightBackgroundColor(-1, -1, -1); // use default color
    option.setTopText("Welcome to the command line option test program.");
    option.setBottomText("Use the WASD keys to navigate, and the Enter key to select an option.");

    option.addOption("function A", []() {
        std::cout << "Function A called." << std::endl;
    });
    option.addOption("function B", []() {
        std::cout << "Function B called." << std::endl;
    });
    option.addOption("function C", nullptr);
    option.addOption("function D", nullptr);
    option.addOption("function E", nullptr);
    option.addOption("function F", nullptr);
    option.addOption("function G", nullptr);
    option.addOption("function H", nullptr);
    option.addOption("function I", nullptr);
    option.addOption("function J", nullptr);

    option.show();
    option.startRecvingInput();

    return 0;
}