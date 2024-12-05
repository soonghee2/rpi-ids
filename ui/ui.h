#ifndef UI_H
#define UI_H

#include <map>
#include <iostream>
#include <iomanip> 
#include <string>
#include <cstring> 
#include <mutex>
#include <cstdarg>
#include <unistd.h>  
#include <termios.h>
#include <set>
#include <vector>
#include <algorithm>
extern std::mutex cout_mutex;
extern std::map<uint32_t, int> id_to_row; 

extern int first_row;    
extern int current_row;    
extern int ID_row;
void updateTop10(uint32_t can_id, int num);
void updateReasonMsg(uint32_t can_id, const std::string& type, const std::string& level, const char* format, ...);
void updateIDMsg(uint32_t can_id, const std::string& type, const std::string& level, const std::string& message, const int num);
void updateAttackMsg(const std::string& type);
int getCursorPosition(int& rows, int& cols);
void initializeAttackTypes();
#endif 
