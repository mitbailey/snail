/**
 * @file snail.c
 * @author Mit Bailey (mitbailey@outlook.com)
 * @brief Single-actuator Non-ASIC Integrating Latching deformable mirror controller board software.
 * @version See Git tags for version information.
 * @date 2023.01.24
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "gpiodev.h"
#include "meb_print.h"

volatile sig_atomic_t done = 0;
void sig_handler(int sig)
{
    done = 1;
}

static char *modestr[] =
    {
        "in      ",
        "out     ",
        "irq_fall",
        "irq_rise",
        "irq_both",
        "error    "};

int exec_file(const char * filename)
{
    int errcode = 0;
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return -1;
    }
    char cmd[100];
    int time[100];
    char *line = NULL;
    int lcount = 0;
    size_t lsize = 0;
    ssize_t rsize = 0;
    while ((rsize = getline(&line, &lsize, fp)))
    {
        int count = 0;
        char *tok = NULL;
        char cmdc = 0;
        int res = 0;
        while ((tok = strtok(line, ",")))
        {
            count += 1; // counter increment
            if (count > 2)
            {
                errcode = -4;
                goto free_resource;
            }
            if (count == 1) // command
            {
                cmdc = tok[0];
                if ((cmdc != 'H') || (cmdc != 'L') || (cmdc != 'W') || (cmdc != 'S'))
                {
                    errcode = -2;
                    goto free_resource;
                }
            }
            if (count == 2) // value
            {
                res = atoi(tok); // convert token to int
                if ((res > 1000) || (res < 1))
                {
                    errcode = -3;
                    goto free_resource;
                }
                break;
            }
        }
        cmd[lcount] = cmdc;
        time[lcount] = res;
        lcount++;
        free(line);
        line = NULL;
    }
    for (int i = 0; i < lcount; i++)
    {
        char cmdc = cmd[i];
        int val = time[i];
    }
free_resource:
    if (line)
        free(line);
    return errcode;
}

int main(int argc, char **argv)
{
    bprintlf("Single-actuator Non-ASIC Integrating Latching deformable mirror controller board software.");
    bprintlf("Copyright (c) 2023 Mit Bailey");
    bprintlf();
    bprintlf("WARNING: This program is designed to control a HIGH VOLTAGE printed circuit board. Attempts to set a GPIO pin to HIGH will prompt for a duration in milliseconds until the pin is automatically set to LOW, with a maximum duration of 1000 ms. User interaction will be denied during the countdown.");
    bprintlf();

    if (argc > 2)
    {
        bprintlf("Usage:");
        bprintlf("sudo %s [Command Filepath]", argv[0]);
        exit(0);
    }

    signal(SIGINT, &sig_handler); // set up signal handler
    int _idx = 1, idx = 976;
    if (gpiodev_pinout == PINOUT_RPI)
    {
        idx = 13;
        _idx = 13;
    }
    else if (gpiodev_pinout == PINOUT_AD9361)
    {
        idx = 976;
        _idx = 1;
    }
    else if (gpiodev_pinout == PINOUT_AD9364)
    {
        idx = 960;
        _idx = 0;
    }

    #define INBUFLEN 8
    char inbuf[INBUFLEN] = {0};
    char in = '\0';

    if (argc == 2)
    {
        // exec_file(argv[1]);
        // exit(0);
        // TODO: exec file (not implemented)
    }

    while (!done)
    {
        int mode = gpioGetMode(_idx);
        if (mode < 0)
            mode = 5;
        printf("\nGPIO: %d | Mode: %s | Value: %d\n", idx, modestr[mode], gpioRead(_idx));
        if (mode == GPIO_OUT)
            printf("[S]elect GPIO, [I]nput mode, set [H]igh, set [L]ow, [Q]uit: ");
        else
            printf("[S]elect GPIO, [O]utput mode, [Q]uit: ");

        memset(inbuf, 0x0, sizeof(inbuf));
        in = '\0';

        bprintf("> ");
        if (fgets(inbuf, sizeof(inbuf), stdin)) {
            if (1 != sscanf(inbuf, "%c", &in)) {
                // in is unsafe
                continue;
            }
        }

        switch (in)
        {
        case 's': // Select GPIO
        case 'S':
            bprintf("Enter GPIO pin number: ");
            char idxbuf[INBUFLEN] = {0};
            memset(idxbuf, 0x0, sizeof(idxbuf));
            if (fgets(idxbuf, sizeof(idxbuf), stdin)) {
                if (1 != sscanf(idxbuf, "%d", &idx)) {
                    // in is unsafe
                    continue;
                }
            }
            if (gpiodev_pinout == PINOUT_RPI)
            {
                _idx = idx;
            }
            else if (gpiodev_pinout == PINOUT_AD9361)
            {
                _idx = idx - 976;
            }
            else if (gpiodev_pinout == PINOUT_AD9364)
            {
                _idx = idx - 960;
            }
            break;

        case 'i': // Input Mode
        case 'I':
            gpioSetMode(_idx, GPIO_IN);
            break;

        case 'o': // Output Mode
        case 'O':
            gpioSetMode(_idx, GPIO_OUT);
            break;

        case 'h': // Set HIGH
        case 'H':
            bprintlf("A duration is required for this action.");
            bprintf("Enter duration in ms (max 1000): ");
            char dinbuf[INBUFLEN] = {0};
            memset(dinbuf, 0x0, sizeof(inbuf));
            int duration = 0;
            if (fgets(dinbuf, sizeof(dinbuf), stdin)) {
                if (1 != sscanf(dinbuf, "%d", &duration)) {
                    // in is unsafe
                    continue;
                }
            }

            if (duration > 1000)
            {
                duration = 1000;
            }

            bprintlf("Pulling pin %d HIGH for a duration of %d ms.", idx, duration);
            bprintlf("User interaction disabled...");
            
            gpioWrite(_idx, GPIO_HIGH);
            
            usleep(duration*1000);
            
            gpioWrite(_idx, GPIO_LOW);
            
            bprintlf("Pulling pin %d LOW.", idx);
            bprintlf("User interaction enabled.");
            break;

        case 'l': // Set LOW
        case 'L':
            gpioWrite(_idx, GPIO_LOW);
            break;

        case 'q': // Quit
        case 'Q':
            done = 1;
            continue;
            break;
        case '\n':
            continue;
            break;
        default:
        {
            break;
        }
        }
    }
    printf("\n\n");
    return 0;
}