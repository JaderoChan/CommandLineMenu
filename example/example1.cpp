#include <command_line_menu.hpp>

int main(int argc, char* argv[])
{
    CommandLineMenu menu;

    menu.setEnableShowIndex(true);
    menu.setEnableAutoAdjustOptionTextWidth(true);
    menu.setOptionTextAlignment(2);
    menu.setMaxColumn(3);
    menu.setTopText("Welcome to the command line menu test program.");
    menu.setBottomText("Use the WASD keys to navigate, and the Enter key to select an option, or the Esc key to exit.");

    menu.addOption("Function A", []()
    {
        std::cout << "Function A called." << std::endl;
        std::cout << "Press any key to back to the main menu." << std::endl;
        CommandLineMenu::getkey();
    });

    menu.addOption("Function B", []()
    {
        std::cout << "Function B called." << std::endl;
        std::cout << "Press any key to back to the main menu." << std::endl;
        CommandLineMenu::getkey();
    });

    int newOptionIndex = 0;
    using DataType1 = std::pair<int*, CommandLineMenu*>;
    DataType1 data1(&newOptionIndex, &menu);
    menu.addOption("Add new", [](void* data)
    {
        auto data_ = static_cast<DataType1*>(data);

        data_->second->addOption("Placeholder " + std::to_string((*data_->first)++), nullptr);
    }, &data1, false);

    menu.addOption("Remove last", [](void* data)
    {
        auto& menu = *static_cast<CommandLineMenu*>(data);

        if (menu.optionCount() > 0)
            menu.removeOption(menu.optionCount() - 1);
    }, &menu, false);

    menu.addOption("Change column", [](void* data)
    {
        auto& menu = *static_cast<CommandLineMenu*>(data);

        std::cout << "Please enter the new column number: ";
        int newColumn = 0;
        std::cin >> newColumn;
        menu.setMaxColumn(newColumn);

    #ifndef _WIN32
        // Discard the LF character.
        CommandLineMenu::getkey();
    #endif // !_WIN32
    }, &menu);

    menu.addOption("Sub Menu", [](void* data)
    {
        auto& menu = *static_cast<CommandLineMenu*>(data);

        CommandLineMenu submenu;
        submenu.setTopText("Sub Menu");

        submenu.addOption("Func 1", []() {
            std::cout << "Hello," << std::endl;
        });

        submenu.addOption("Func 2", []() {
            std::cout << "World!" << std::endl;
        });

        submenu.addOption("Placeholder", nullptr);

        submenu.show();
        submenu.startReceiveInput();
    }, &menu);

    menu.addOption("Exit", [](void* data)
    {
        auto& menu = *static_cast<CommandLineMenu*>(data);

        menu.endReceiveInput();
    }, &menu, false);

    menu.show();
    menu.startReceiveInput();

    return 0;
}
