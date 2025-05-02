#include <libpynq.h>
#include "displayfunc.h" // Replace with your actual display header
#include <display.h>     // Replace with your actual display header
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> //for printf
#include <stdint.h>

void displayUncombinedText(display_t display,unsigned char text[],unsigned int xpos,unsigned int ypos, uint16_t bgcolor){
  uint8_t buffer_fx16G[FontxGlyphBufSize]; // Font stuff
  uint8_t fontWidth_fx16G, fontHeight_fx16G;
  FontxFile fx16G[2];
  InitFontx(fx16G, "../../fonts/ILGH16XB.FNT", "");
  GetFontx(fx16G, 0, buffer_fx16G, &fontWidth_fx16G, &fontHeight_fx16G);
   
  displayDrawFillRect(&display,xpos,ypos-13,239,ypos,bgcolor);
  displayDrawString(&display,fx16G,xpos,ypos,text,2016);
}

void matrix_display(display_t *display, FontxFile *fx16G, uint8_t (*matrix)[5], int size, int gap, int *current_pos){
 
   int x_margin = (240-4*gap-5*size)/2, y_margin =(240-4*gap-5*size)/2;
   displayDrawFillRect(display,x_margin - gap/2, y_margin - gap/2, x_margin+5*size+4*gap + gap/2, y_margin+5*size+4*gap + gap/2, RGB_WHITE);
   for(int i = 0; i < 5; i++){
     for(int j = 0; j < 5; j++){
      displayDrawFillRect(display,(j*(size+gap)+x_margin),(i*(size+gap)+y_margin),(j*(size+gap)+x_margin+size),(i*(size+gap)+y_margin+size), rgb_conv(0,256 - matrix[i][j],256 - matrix[i][j]));
      if(i == 0){
        char text[8];
        sprintf(text,"F%d", j+1);
        displaySetFontDirection(display, TEXT_DIRECTION0);
        displayDrawString(display, fx16G, (j*(size+gap)+x_margin+size/4) , (i*size+y_margin-gap/2) ,(uint8_t*)text,RGB_WHITE );
        }
      if(j == 0){
	       char text[8];
         sprintf(text, "A%d", i+1);
	       displaySetFontDirection(display, TEXT_DIRECTION270);
         displayDrawString(display, fx16G, (j*size+x_margin-gap/2), (i*(size+gap)+y_margin + size/1.3), (uint8_t*)text, RGB_WHITE);
      }
    }
  }
  displayDrawFillCircle(display, (current_pos[1]*(size+gap)+x_margin+size/2), (current_pos[0]*(size+gap)+y_margin+size/2), size/4, RGB_RED );
}


void sendToMotor(int row, int col) {
    fflush(stdout);
    printf("Sending to motor: Row=%d, Col=%d\n", row, col);
    
    switchbox_set_pin(IO_AR2, SWB_UART0_TX);
    switchbox_set_pin(IO_AR3, SWB_UART1_TX); // Or appropriate pin for UART1 TX

    uart_init(UART0);
    uart_reset_fifos(UART0);
    uart_init(UART1);
    uart_reset_fifos(UART1);

    //Since uart_send returns void, we need alternative error handling
    //1. Check for space before sending:
    if (!uart_has_space(UART0)) {
        printf("UART0 buffer full!  Row data not sent.\n");
        //Consider adding retry mechanism or error handling here.
    } else {
        uart_send(UART0, row);
        printf("Sent row (%d) to UART0\n", row);
    }

    if (!uart_has_space(UART1)) {
        printf("UART1 buffer full! Col data not sent.\n");
        //Consider adding retry mechanism or error handling here.
    } else {
        uart_send(UART1, col);
        printf("Sent col (%d) to UART1\n", col);
    }
     // 10ms delay - adjust as needed
    return;
}

bool isValid(int row, int col) {
    return row >= 0 && row < 5 && col >= 0 && col < 5;
}

int main() {
    pynq_init();
    switchbox_init();
    display_t display;
    display = initDisplay(0);

    uint8_t introText[50] ="---- Algorithm Submodule ----";

    displayUncombinedText(display,introText,5,15,0);

    int stressMatrix[5][5] = {0}; // 0: unvisited, 1: visited, -1: blocked
    stressMatrix[4][4] = 1;

    restart:
    int currentRow = 4;
    int currentColumn = 4; // Columns are freq, Rows are Amplitudes
    uint8_t A[5] = "A: ";
    uint8_t F[5] = "F: ";
    uint8_t Cstate[16]= "Current State: ";

    displayText(display,A,(float)currentRow,5,30,0);
    displayText(display,F,(float)currentColumn,5,45,0);
    printf("A%d F%d",currentRow,currentColumn);

    float lastHb,lastCry;
    lastHb = 216;
    lastCry = 100;

    sleep_msec(1000);
    sendToMotor(4,4);
    sleep_msec(1000);

    char state = ' ';
    char up = 'd';

    while (true) {

        if(!isValid(currentRow,currentColumn)){
            currentColumn=4;
        }
        //Logic
        if(currentColumn == 0 && currentRow == 0){
            displayText(display,A,(float)currentRow,5,30,0);
            displayText(display,F,(float)currentColumn,5,45,0);
            printf("A%d F%d",currentRow,currentColumn);
            break;
        }

        currentColumn-=1;
        if(stressMatrix[currentRow][currentColumn] == -1){
            currentRow-=1;
            currentColumn+=2;
            continue;
        }
        
        
        //Display stuff
        displayText(display,A,(float)currentRow,5,30,0);
        displayText(display,F,(float)currentColumn,5,45,0);
        //snprintf(coordsText, sizeof(coordsText), "A%d F%d",currentRow,currentColumn);
        //displayUncombinedText(display,coordsText,5,15,0);
        printf("A: %d F: %d",currentRow,currentColumn);


        sendToMotor(currentRow,currentColumn);
        sleep_msec(100); // Decide on this after testing

        switchbox_set_pin(IO_AR0, SWB_UART0_RX); //will be reciever of signal 0
        switchbox_set_pin(IO_AR1, SWB_UART1_RX); //will be reciever of signal 1

        uart_init(UART0); //initalizes the uart0 signal which the row
        uart_reset_fifos(UART0); //cleans the uart0 signal
        uart_init(UART1); //initalizes the uart1 signal which is column
        uart_reset_fifos(UART1); //cleans the uart1 signal
    
        int currentHb,currentCry;

        printf("Enter HeartBeat value: ");
        scanf("%d",&currentHb);//uart_recv(UART0);
        if(currentHb == 255){
            currentColumn+=1;
            continue;
        }
        printf("Recieved Hb %d\n", currentHb);
        //currentCry = uart_recv(UART1);
        printf("Enter Cry Volume value: ");
        scanf("%d",&currentCry);
        if(currentCry == 255){
            if(currentColumn < 4){
                currentColumn+=1;
            }
            continue;
        }
        printf("Recieved Cry %d\n", currentCry);

        if(up == 'u'){
            up = 'd';
            currentRow-=1;
            currentColumn+=1;
            lastHb = currentHb;
            lastCry = currentCry;
            printf("u A: %d F: %d",currentRow,currentColumn);
            continue;
        }

        char metric;

        if(currentHb< 140){
            metric = 'c';
        } else{
            metric = 'h';
        }

        printf("Current stress: %c\n",metric);

        if(metric == 'c'){
            if (currentCry < lastCry-lastCry*5/100){
                state = 'd';
            } else if (currentCry > lastCry+lastCry*5/100){
                state = 'i';
            } else {
                state = 's';
            }
        } else if(metric == 'h'){
            if (currentHb < lastHb-lastHb*5/100){
                state = 'd';
            } else if (currentHb > lastHb + lastHb*5/100){
                state = 'i';
            }else if (currentHb>=195 && currentColumn!=3 && currentRow!=4){
                state = 'p';
            } else{
                state ='s';
            }
        }

        displayText(display,Cstate,state,5,60,0);
        printf("Current state: %c\n",state);
        if(state == 'd'){
            stressMatrix[currentRow][currentColumn] = 1;
        } else if(state == 'i'){
            stressMatrix[currentRow][currentColumn]=-1;
            currentColumn+=2;
            up = 'u';
        } else if(state == 'p'){
            stressMatrix[currentRow][currentColumn]=-1;

            for(int F = 0; F < 5; F++){
                printf("\n");
                for(int A = 0; A < 5; A++){
                    printf(" %d   |", stressMatrix[F][A]);
                }
            }
            printf("\n");

            goto restart;
        } else if(state == 's'){
            stressMatrix[currentRow][currentColumn]=-1;
            currentColumn+=2;
            printf("s A: %d F: %d",currentRow,currentColumn);
            up = 'u';
        }

        lastHb = currentHb;
        lastCry = currentCry;
        FontxFile fx16G[2];
        int test_positions[2] = {4,4};

        matrix_display(&display, fx16G ,stressMatrix, 22, 4, test_positions);


        for(int F = 0; F < 5; F++){
            printf("\n");
            for(int A = 0; A < 5; A++){
                printf(" %d   |", stressMatrix[F][A]);
            }
        }
        printf("\n");

    }

    
    printf("\n");

    closeDisplay(display);
    switchbox_destroy();
    pynq_destroy();
    return 1;

    //restart: ; Marked by AI
}
