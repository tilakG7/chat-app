#include <mutex>
#include <string>

using namespace std;

/**
* Singleton class to prompt and read information from the user.
* Also supports writing to the terminal.
* User promt is always shown as the last line in the terminal, 
* and text written to the terminal is displayed above it.
*/
class Console {
public:
    // Initializes the Console singleton on first call
    // @return - reference to Console singleton object
    static Console& getInstance();

    /**
    * Prompts the user and waits for user to input data and hit "enter"
    * @note This is a blocking function
    * @param prompt - a message to prompt the user with. Ex: "Enter your age>"
    * @return the string the user inputted. Ex: "My age is 43"
    */
    string read(const string &prompt);

    /**
    * Writes data to the console, without overwriting the user prompt and user
    * input
    * @param s - string to write to the console
    */
    void write(const string &s);

    // Delete copy constructor, copy assignment and move assignment
    Console(Console &) = delete;
    Console& operator=(const Console &) = delete;
    Console&& operator=(Console &&) = delete;
private:

    // private constructor to support Singleton pattern
    Console();

    // Values for special ASCII characters
    static constexpr int kDelete = 0x7F; 
    static constexpr int kNewLine = 0x0A; 
    mutex m_console_mtx;   // mutex for ownership of shared member variables and cout
    string m_prompt;     // prompt to display to the user
    string m_user_input; // store what the user has typed so far
};