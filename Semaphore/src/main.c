#include <console/console.h>
#include <string.h>
#include <sys/printk.h>
#include <version.h>
#include <zephyr.h>

#define GO_MSG "Go"
#define TIMEOUT_MSG "Timeout"
#define STOP_MSG "Stop"

typedef enum {
  kRedLight = 0,
  kYellowLight = 1,
  kGreenLight = 2,
  amount_lights
} tLight;

typedef enum {
  kRedState = 0,
  kYellowState = 1,
  kGreenState = 2,
  amount_states,
} tState;

typedef struct sStateTableEntry {
  tLight light;        // all states have associated lights
  tState goEvent;      // state to enter when go event occurs
  tState stopEvent;    // ... when stop event occurs
  tState timeoutEvent; // ... when timeout occurs
} sStateTableEntry;

char *light_to_string(tLight light) {
  char *s = "";
  switch (light) {
  case kRedLight:
    s = "Red";
    break;
  case kGreenLight:
    s = "Green";
    break;
  case kYellowLight:
    s = "Yellow";
    break;
  default:
    break;
  }

  return s;
}

void LightOff(tLight light) {
  printk("%s light is off.\n", light_to_string(light));
}

void LightOn(tLight light) {
  printk("%s light is on.\n", light_to_string(light));
}

static const sStateTableEntry stateTable[] = {
    [kRedState] =
        {
            .light = kRedLight,
            .goEvent = kGreenState,
            .stopEvent = kRedState,
            .timeoutEvent = kRedState,
        },
    [kYellowState] =
        {
            .light = kYellowLight,
            .goEvent = kYellowState,
            .stopEvent = kYellowState,
            .timeoutEvent = kRedState,
        },
    [kGreenState] =
        {
            .light = kGreenLight,
            .goEvent = kGreenState,
            .stopEvent = kYellowState,
            .timeoutEvent = kGreenState,
        },
};

// Go event handler
tState HandleEventGo(tState currentState) {
  printk("Handling Go event.\n");
  tState new_state;
  LightOff(stateTable[currentState].light);
  new_state = stateTable[currentState].goEvent;
  LightOn(stateTable[new_state].light);
  return new_state;
}

// Stop event handler
tState HandleEventStop(tState currentState) {
  printk("Handling Stop event.\n");
  tState new_state;
  LightOff(stateTable[currentState].light);
  new_state = stateTable[currentState].stopEvent;
  LightOn(stateTable[new_state].light);
  return new_state;
}

// Timeout event handler
tState HandleEventTimeout(tState currentState) {
  printk("Handling Timeout event.\n");
  tState new_state;
  LightOff(stateTable[currentState].light);
  new_state = stateTable[currentState].timeoutEvent;
  LightOn(stateTable[new_state].light);
  return new_state;
}

void main(void) {
  printk("Hello! I'm using Zephyr %s on %s, a %s board. \n\n",
         KERNEL_VERSION_STRING, CONFIG_BOARD, CONFIG_ARCH);

  console_getline_init();
  printk("Enter a line finishing with Enter:\n");

  tState currentState = kGreenState;
  LightOn(kGreenLight);
  LightOff(kRedLight);
  LightOff(kYellowLight);

  while (1) {
    printk("Type an event (Go, Stop, Timeout) > ");
    char *s = console_getline();

    if (!strcmp(s, GO_MSG)) {
      currentState = HandleEventGo(currentState);
      continue;
    }

    if (!strcmp(s, TIMEOUT_MSG)) {
      currentState = HandleEventTimeout(currentState);
      continue;
    }

    if (!strcmp(s, STOP_MSG)) {
      currentState = HandleEventStop(currentState);
      continue;
    }
  }
}