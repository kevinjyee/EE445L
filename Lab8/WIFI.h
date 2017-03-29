/* File Name:    WIFI.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      March 29, 2017 by Stefan Bordovsky
 * Description:  This program contains utilities for playing with WIFI.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 3/29/2017	
 */
 
 #include <stdint.h>
 
 // ***************** Update_User_Data ****************
// Establishes connection with server and updates it with user info.
// Inputs:  none
// Outputs: none
void Update_User_Data(uint32_t steps_taken, uint32_t avg_speed);