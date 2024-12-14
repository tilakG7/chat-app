#include "console.h"

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;


Console& Console::getInstance() {
    static Console c;
    return c;
}

string Console::read(const string &prompt) {
    {
        unique_lock<mutex> lck{m_console_mtx};
        m_prompt = prompt;
        std::cout << m_prompt;
    }

    while(true) {
        int next_char = getchar(); // get next character from user
    
        unique_lock<mutex> lck{m_console_mtx};

        bool user_done_typing = false;
        // modify the string containing user data according to input
        switch(next_char) {
            case kDelete:
                // only erase output if it exists
                if(m_user_input.size()) {
                    m_user_input.pop_back();
                    // move cursor back one character
                    // overwrite character with empty space
                    // move cursor back one character again
                    std::cout << "\b \b"; 
                }
                break;
            case kNewLine:
                user_done_typing = true;
                break;
            default:
                m_user_input.append(1, static_cast<char>(next_char)); // append 1 next_char
                std::cout << static_cast<char>(next_char);
        }

        if(user_done_typing) {
            std::cout << std::endl;
            string temp = m_user_input;
            m_user_input.clear();
            m_prompt.clear();
            return temp;
        }
    }
}

void Console::write(const string &s) {
    {
        unique_lock<mutex> lck(m_console_mtx);
        
        const size_t current_input_size = m_prompt.size() + m_user_input.size();
        
        // no need to overwrite user input line
        if(!current_input_size) {
            std::cout << s << std::endl;
            return;
        }

        // overwrite user input line and reprint user input line
                    // move cursor to the start of the line
        std::cout << '\r'
                    // set overwrite the remaining line with empty character space
                    << std::setfill(' ') 
                    << std::setw(current_input_size) 
                    << ""
                    // move cursor to start of line again
                    << '\r';
        std::cout << std::setw(0)
                    // write the intended message
                    << s
                    << std::endl
                    // write the user prompt and input
                    << m_prompt 
                    << m_user_input
                    << std::flush;
    }
}

Console::Console() {
    struct termios oldattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    struct termios newattr = oldattr;
    // clear ICANON and ECHO flag
    // ICANON - input is only given to file once "enter" has been pressed
    // ECHO   - input is written back to the terminal
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
}
    

// void printer(Console &c) {
//     while(true) {
//         c.write("hello");
//         std::this_thread::sleep_for(1s);
//     }
// }

// void getUserInput(Console &c) {
//     while(true) {
//         string user_input = c.read(">");
//         c.write("User typed: " + user_input);
//     }
// }

// int main() {
//     thread t1(printer, std::ref(Console::getInstance()));
//     thread t2(getUserInput, std::ref(Console::getInstance()));
//     t1.join();
//     t2.join();
//     return 0;
// }