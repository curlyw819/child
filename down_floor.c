#include <signal.h>
#include <stdint.h>
#include <time.h>
#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"
#include <unistd.h>
void point_to_pattern(int p[9][9]);
uint8_t row_pattern[9];
volatile sig_atomic_t stopped = 0;
void int_handler(int sig) {
    printf("Received signal %d\n", sig);
    stopped = 1;
}
mraa_gpio_context pin_load, pin_din, pin_clk;
void send_byte(uint8_t d) {
    for (int i = 7; i >= 0; --i) {
        mraa_gpio_write(pin_clk, 0);
        delay_ns(1000);
        mraa_gpio_write(pin_din, (d >> i) & 1u);
        delay_ns(1000);
        mraa_gpio_write(pin_clk, 1);  //
        delay_ns(1000);
  }
}
void write_reg(uint8_t addr, uint8_t data) {
    mraa_gpio_write(pin_clk, 1);
    mraa_gpio_write(pin_load, 0); //
    send_byte(addr);
    send_byte(data);
    mraa_gpio_write(pin_clk, 0);
    mraa_gpio_write(pin_load, 1);
    delay_ns(1000);
}
void init_matrix() {
  // Refer to the datasheet for the meaning of these numbers

    write_reg(0x0c, 0x01);  // leave shutdown mode
    write_reg(0x0f, 0x00);  // turn off display test
    write_reg(0x09, 0x00);  // set decode mode: no decode
    write_reg(0x0b, 0x07);  // set scan limit to full
    write_reg(0x0a, 0x01);  // set brightness (duty cycle = 3/32) //
}
int main(){
    /*最右邊的按鈕*/
    mraa_gpio_context gpio;
    gpio = mraa_gpio_init(22);
    mraa_gpio_dir(gpio, MRAA_GPIO_IN);
    /**/

    /*按鈕*/
    int switch_status[8] = {0};
    // PL: asynchronous parallel load input (active LOW)
    mraa_gpio_context pin_pl = mraa_gpio_init(UP_HAT_74HC165_PL);
    // Q7: serial output from the last stage
    mraa_gpio_context pin_data = mraa_gpio_init(UP_HAT_74HC165_Q7);
    // CE: clock enable input (active LOW)
    mraa_gpio_context pin_ce = mraa_gpio_init(UP_HAT_74HC165_CE);
    // CP: clock input (LOW-to-HIGH edge-triggered)
    mraa_gpio_context pin_clk_2 = mraa_gpio_init(UP_HAT_74HC165_CP);
    mraa_gpio_dir(pin_pl, MRAA_GPIO_OUT);
    mraa_gpio_dir(pin_data, MRAA_GPIO_IN);
    mraa_gpio_dir(pin_ce, MRAA_GPIO_OUT);
    mraa_gpio_dir(pin_clk_2, MRAA_GPIO_OUT);
    /**/

    /*matrix*/
    pin_load = mraa_gpio_init(UP_HAT_MAX7219_LOAD);
    pin_din = mraa_gpio_init(UP_HAT_MAX7219_DIN);
    pin_clk = mraa_gpio_init(UP_HAT_MAX7219_CLK);
    if (!(pin_load && pin_din && pin_clk && pin_clk_2)) {
        fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
        return EXIT_FAILURE;
    }
    mraa_gpio_dir(pin_load, MRAA_GPIO_OUT);
    mraa_gpio_dir(pin_din, MRAA_GPIO_OUT);
    mraa_gpio_dir(pin_clk, MRAA_GPIO_OUT);
    init_matrix();
    /**/

    /*變數宣告*/
    for (int i = 1; i <= 8; i++)
        row_pattern[i] = 0b00000000;
  
    int point[9][9];
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            point[i][j] = 0;

    int point_temp[9][9];
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            point_temp[i][j] = 0;
    signal(SIGINT, int_handler);
    int count = 8;
    int n;
    int col[9] = {0};
    int line[9] = {0};
    line[1] = 8;
    line[2] = 9;
    line[3] = 10;
    line[4] = 11;
    line[5] = 12;
    line[6] = 13;
    line[7] = 14;
    line[8] = 15;


    int child_row = 0;
    int child_col = 4;

    /*debounce 變數宣告*/
    clock_t Current_time, Previous_down_time, Previous_up_time;
    Previous_down_time = 0;  // Assuming last down time is so long ago
    Previous_up_time = 0;
    int status = 1;
    int right_down = 0;
    int right_up = 0;
    int right_lock = 1;

    clock_t left_Current_time, left_Previous_down_time, left_Previous_up_time;
    left_Previous_down_time = 0;  // Assuming last down time is so long ago
    left_Previous_up_time = 0;
    int left_status = 1;
    int left_down = 0;
    int left_up = 0;
    int left_lock = 1; 

    int back_groung_timer = 100;
    int f = 0; // 0表背景有更新 1表背景沒更新
    int row_previous = 0;
    int stay = 0; // 小朋友有沒有停在樓梯上


    while (!stopped) {



        
        /*
        if(child_row == 0)
            child_row = 1;
        */
        if(stay == 0){
            child_row++;

            if(child_row >= 1 && child_row <= 9)
                row_previous = child_row - 1;
            /*
            if(child_row == 9)
                child_row = 1;            
            */
        }

        if(child_row == 9 || child_row == 0){ // 遊戲結束 重頭開始

            for (int i = 1; i <= 8; i++)
                row_pattern[i] = 0b00000000;
            for (int i = 1; i <= 8; i++)
                for (int j = 1; j <= 8; j++)
                    point[i][j] = 0;
            for (int i = 1; i <= 8; i++)
                for (int j = 1; j <= 8; j++)
                    point_temp[i][j] = 0;

            count = 8;
            for(int i = 0; i < 9; i++)
                col[9] = 0;
            line[1] = 8;
            line[2] = 9;
            line[3] = 10;
            line[4] = 11;
            line[5] = 12;
            line[6] = 13;
            line[7] = 14;
            line[8] = 15;
            child_row = 1;
            child_col = 4;
            Previous_down_time = 0;  
            Previous_up_time = 0;
            status = 1;
            right_down = 0;
            right_up = 0;
            right_lock = 1;
            left_Previous_down_time = 0; 
            left_Previous_up_time = 0;
            left_status = 1;
            left_down = 0;
            left_up = 0;
            left_lock = 1;
            back_groung_timer = 100;
            f = 0;
            row_previous = 0;
            stay = 0;


        }



        back_groung_timer--;
        if(back_groung_timer == -1)
            back_groung_timer = 99;
        if(back_groung_timer % 2 == 0){ // 減慢背景更新速度


            
            /*樓梯start*/
            srand( time(NULL) ); // 亂數種子s
            /*
            n = rand() % 6 + 1;
            col[8] = n;
            */
            while(1){
                n = rand() % 6 + 1;
                col[8] = n;
                if(col[6] == n){
                    continue;
                }
                else{
                    break;
                }
            }


            for(int i = 1; i <= 8; i++)
                if(line[i] == 0)
                    line[i] = 8;
            

            if(line[1] <= 8){
                point[line[1]][col[count]] = 1;
                point[line[1]][col[count] + 1] = 1;
                /*
                if(child_row == line[1] && (child_col == col[count] || child_col == col[count] + 1)){
                    printf("match\n");
                }
                */
                //point[line[1]][col[count] + 2] = 1;
            }
            
            /*
            if(line[2] <= 8){
                int temp_2 = (count % 8 + 1) % 8;
                if(temp_2 == 0)
                    temp_2 = 8;
                point[line[2]][col[temp_2]] = 1;
                point[line[2]][col[temp_2] + 1] = 1;
                //point[line[2]][col[temp_2] + 2] = 1;
            
            }
            */
            
            if(line[3] <= 8){
                
                int temp_3 = (count % 8 + 2) % 8;
                if(temp_3 == 0)
                    temp_3 = 8;
                
                point[line[3]][col[temp_3]] = 1;
                point[line[3]][col[temp_3] + 1] = 1;
                /*
                if(child_row == line[3] && (child_col == col[temp_3] || child_col == col[temp_3] + 1)){
                    printf("match\n");
                }
                */
                //point[line[3]][col[temp_3] + 2] = 1;
            
            }
            
            /*
            if(line[4] <= 8){
                int temp_4 = (count % 8 + 3) % 8;
                if(temp_4 == 0)
                    temp_4 = 8;
                point[line[4]][col[temp_4]] = 1;
                point[line[4]][col[temp_4] + 1] = 1;
                //point[line[4]][col[temp_4] + 2] = 1;
            }
            */

            if(line[5] <= 8){
                
                int temp_5 = (count % 8 + 4) % 8;
                if(temp_5 == 0)
                    temp_5 = 8;
                

                point[line[5]][col[temp_5]] = 1;
                point[line[5]][col[temp_5] + 1] = 1;
                /*
                if(child_row == line[5] && (child_col == col[temp_5] || child_col == col[temp_5] + 1)){
                    printf("match\n");
                }              
                */  
                //point[line[5]][col[temp_5] + 2] = 1;
            }
            
            /*
            if(line[6] <= 8){
                int temp_6 = (count % 8 + 5) % 8;
                if(temp_6 == 0)
                    temp_6 = 8;
                point[line[6]][col[temp_6]] = 1;
                point[line[6]][col[temp_6] + 1] = 1;
                //point[line[6]][col[temp_6] + 2] = 1;
            }
            */
            
            if(line[7] <= 8){
                
                int temp_7 = (count % 8 + 6) % 8;
                if(temp_7 == 0)
                    temp_7 = 8;
                
                point[line[7]][col[temp_7]] = 1;
                point[line[7]][col[temp_7] + 1] = 1;
                /*
                if(child_row == line[7] && (child_col == col[temp_7] || child_col == col[temp_7] + 1)){
                    printf("match\n");
                }     
                */
                //point[line[7]][col[temp_7] + 2] = 1;
            }
            
            /*
            if(line[8] <= 8){
                int temp_8 = (count % 8 + 7) % 8;
                if(temp_8 == 0)
                    temp_8 = 8;
                point[line[8]][col[temp_8]] = 1;
                point[line[8]][col[temp_8] + 1] = 1;
                //point[line[8]][col[temp_8] + 2] = 1;
            }
            
            */
            count--;
            if (count == 0)
                count = 8;
            for(int i = 1; i <= 8; i++){
                line[i]--;   
            }
            /*col設定值上移*/
            col[0] = col[1];
            col[1] = col[2];
            col[2] = col[3];
            col[3] = col[4];
            col[4] = col[5];
            col[5] = col[6];
            col[6] = col[7];
            col[7] = col[8];

            /*樓梯end*/
            f = 0;
        }
        else{
            f = 1;
        }



        

        /*按鈕and小朋友 start*/
        int counter = 0;
        int match = 0;
        while(counter <= 100){
            counter++;
            /*按鈕初始化 start*/
            mraa_gpio_write(pin_ce, 1);  // disable clk input
            delay_ns(1000);
            mraa_gpio_write(pin_pl, 0);  // enable parallel data input
            delay_ns(1000);
            mraa_gpio_write(pin_pl, 1);  // disable & hold parallel data input
            delay_ns(1000);
            mraa_gpio_write(pin_ce, 0);  // enable clk input
            delay_ns(1000);
            for (int i = 0; i < 8; i++) {
                mraa_gpio_write(pin_clk_2, 0);  // unset clk signal
                delay_ns(1000);

                switch_status[i] = mraa_gpio_read(pin_data);  // read one bit data from pin_data
                mraa_gpio_write(pin_clk_2, 1);   // set clk signal
                delay_ns(1000);
            }
            mraa_gpio_write(pin_clk_2, 0);  // unset clk signal
            delay_ns(1000);
            /*按鈕初始化 end*/



            /*switch_status[5]按鈕debounce start*/
            
            int debounce_temp = 0;

            while (debounce_temp <= 100) {
                debounce_temp++;
                if (switch_status[5] == 0) {         
                    Current_time = clock();
                    if (((double)Current_time - Previous_down_time) / CLOCKS_PER_SEC > 0.01) {
                        if (status == 1) {
                            right_down = 1;
                            status = 0;
                        }
	                    else{
	                        Previous_down_time = clock();
	                    }
                    }
                } 
    
                else {
                    Current_time = clock();
                    if (((double)Current_time - Previous_up_time) / CLOCKS_PER_SEC > 0.01) {
                        if (status == 0) {
                            right_up = 1;
                            right_lock = 1;
                            status = 1;
                        }
	                    else{
	                        Previous_up_time = clock();
	                    }
                    }
      
                }

            }
            /*switch_status[5]按鈕debounce end*/


            /*switch_status[6]按鈕debounce start*/
            
            debounce_temp = 0;
            while (debounce_temp <= 100) {

                debounce_temp++;
                if (switch_status[6] == 0) {         
                    left_Current_time = clock();
                    if (((double)left_Current_time - left_Previous_down_time) / CLOCKS_PER_SEC > 0.01) {
                        if (left_status == 1) {
                            left_down = 1;
                            left_status = 0;
                        }
	                    else{
	                        left_Previous_down_time = clock();
	                    }
                    }
                } 
    
                else {
                    left_Current_time = clock();
                    if (((double)left_Current_time - left_Previous_up_time) / CLOCKS_PER_SEC > 0.01) {
                        if (left_status == 0) {
                            left_up = 1;
                            left_lock = 1;
                            left_status = 1;
                        }
	                    else{
	                        left_Previous_up_time = clock();
	                    }
                    }
      
                }

            }
            /*switch_status[5]按鈕debounce end*/

            /*按鈕and小朋友 start*/
            
            if(right_down == 1 && right_up == 1){
                right_down = 0;
                right_up = 0;
            }
            if(left_down == 1 && left_up == 1){
                left_down = 0;
                left_up = 0;
            }
            /*
            if(right_down == 1 && right_lock == 1 && point_temp[child_row][child_col + 1] == 1){
                printf("match\n");
            }
            if(right_down == 1 && right_lock == 1 && point[child_row][child_col + 1] == 1){
                printf("match\n");
            }
            if(left_down == 1 && left_lock == 1 && point_temp[child_row][child_col + 1] == 1){
                printf("match\n");
            }
            if(left_down == 1 && left_lock == 1 && point[child_row][child_col - 1] == 1){
                printf("match\n");
            }
            */
            if(right_down == 1 && right_lock == 1 && point_temp[child_row - 1][child_col + 1] == 1 && stay == 0){
                match = 1;
            }
            if(right_down == 1 && right_lock == 1 && point[child_row - 1][child_col + 1] == 1 && stay == 0){
                match = 1;
            }
            if(left_down == 1 && left_lock == 1 && point_temp[child_row - 1][child_col - 1] == 1 && stay == 0){
                match = 1;
            }
            if(left_down == 1 && left_lock == 1 && point[child_row - 1][child_col - 1] == 1 && stay == 0){
                match = 1;
            }


            
            if(right_down == 1 && right_lock == 1 && !(right_down == 1 && right_lock == 1 && point[child_row][child_col + 1] == 1) && !(right_down == 1 && right_lock == 1 && point_temp[child_row][child_col + 1] == 1) && !match){
                
                right_lock = 0;
                point[child_row][child_col] = 0;
                point_temp[child_row][child_col] = 0;
                child_col++;
                if(child_col == 9)
                    child_col = 1;
                if(f == 0 && stay == 1 && point_temp[child_row + 1][child_col] == 0 && point[child_row + 1][child_col] == 0){
                    child_row++;
                }
            }
            
            /*
            else if(right_down == 1 && right_lock == 1 && ((point_temp[child_row + 1][child_col] == 1 && point_temp[child_row + 1][child_col + 1] == 1) || (point[child_row + 1][child_col] == 1 && point[child_row + 1][child_col + 1] == 1))){
                right_lock = 0;
                point[child_row][child_col] = 0;
                point_temp[child_row][child_col] = 0;
                child_col++;
                if(child_col == 9)
                    child_col = 1; 
            }
            */



            if(left_down == 1 && left_lock == 1 && !(left_down == 1 && left_lock == 1 && point[child_row][child_col - 1] == 1) && !(left_down == 1 && left_lock == 1 && point_temp[child_row][child_col - 1] == 1) && !match){
                left_lock = 0;
                point[child_row][child_col] = 0;
                point_temp[child_row][child_col] = 0;
                child_col--;
                if(child_col == 0)
                    child_col = 8;
                if(f == 0 && stay == 1 && point_temp[child_row + 1][child_col] == 0 && point[child_row + 1][child_col] == 0){
                    child_row++;
                }               
            }
            /*
            else if(left_down == 1 && left_lock == 1 &&  ((point_temp[child_row + 1][child_col] == 1 && point_temp[child_row + 1][child_col - 1] == 1) || (point[child_row + 1][child_col] == 1 && point[child_row + 1][child_col - 1] == 1))){
                left_lock = 0;
                point[child_row][child_col] = 0;
                point_temp[child_row][child_col] = 0;
                child_col--;
                if(child_col == 0)
                    child_col = 8;  
            }
            */



            


            


            point[child_row][child_col] = 1;
            if(stay == 0){ // 沒有停下來才要去除前一步驟的小朋友
                
                point[row_previous][child_col] = 0;
                
                point_temp[row_previous][child_col] = 0;
                
                
            }
            
            if(f == 1){

                point_temp[child_row][child_col] = 1;
            }



        }



        /*按鈕and小朋友 end*/
        
        /*小朋友stay start*/
        
        if(point[child_row + 1][child_col] == 1 || point_temp[child_row + 1][child_col] == 1){
            stay = 1;
        }
        if(point_temp[child_row + 1][child_col] == 1 || point_temp[child_row + 2][child_col] == 1){
            child_row--;
        }


        if(point[child_row + 1][child_col] == 0 && point_temp[child_row + 1][child_col] == 0){
            stay = 0;
        }


        /*小朋友stay end*/

        
        

        

        if(f == 1){ // 在背景沒更新時
            point_to_pattern(point_temp);
            for (int i = 1; i <= 8; ++i) {
                write_reg(i, row_pattern[i]);
            }
            for(int i = 1; i <= 8; i++){
                for(int j = 1; j <= 8; j++){
                    point_temp[i][j] = 0;
                }
            }

        }

        if(f == 0){// 在背景有更新時
            for (int i = 1; i <= 8; i++)
                for (int j = 1; j <= 8; j++)
                    point_temp[i][j] = point[i][j];

            point_to_pattern(point);
            for (int i = 1; i <= 8; ++i) {
                write_reg(i, row_pattern[i]);
            }
            for(int i = 1; i <= 8; i++){
                for(int j = 1; j <= 8; j++){
                    point[i][j] = 0;
                }
            }
        }


        //delay_ms(100);
    }
    /* release resource section */
    for (int i = 1; i <= 8; ++i) {
        write_reg(i, 0x00);
    }
    mraa_gpio_close(pin_load);
    mraa_gpio_close(pin_din);
    mraa_gpio_close(pin_clk);

    /*switch*/
    mraa_gpio_close(pin_pl);
    mraa_gpio_close(pin_data);
    mraa_gpio_close(pin_ce);
    mraa_gpio_close(pin_clk_2);
    /**/
    
}
void point_to_pattern(int p[9][9]){
    int i, j;
    int k = 1;
    uint8_t sum = 0;
    for(i = 1; i <= 8; i++){
        for(j = 8; j >= 1; j--){
            sum = sum + p[i][j]*k;
            k = k * 2;
        }
        row_pattern[i] = sum;
        sum = 0;
        k = 1;
  }
}
