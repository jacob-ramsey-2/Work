/**
 * Jacob Ramsey
 * main.c
 *
 * This is the only file for the Work program. Thanks for indulging in my relentless over automation.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// MAX VALUES
#define MAX_TASK_LENGTH 100
#define MAX_TASK_COUNT 50
#define MAX_FILE_COUNT 20
#define MAX_FILE_LENGTH 100
#define MAX_FLAG_COUNT 10
#define MAX_ERROR_LENGTH 100

// CONSTANTS
#define bool char
#define FALSE 0
#define TRUE 1
#define NOT_FOUND -1
#define NUM_FLAGS 1

// MACROS
#define FLAG_ERROR(x) printf("Uknown Flag: %s\n", x)
#define WORKFILE_ERROR(x) printf("Workfile error: %s\n", x)

// ENUMS
typedef enum {
	COMMENT,
	TASK,
	TASK_FILE,
	TASK_FLAG,
	BLANK,
	OTHER
} LineState;

// STRUCTS
typedef struct {
	char name[MAX_TASK_LENGTH];
	char files[MAX_FILE_COUNT][MAX_FILE_LENGTH];
	char flags[MAX_FLAG_COUNT][2];
	unsigned int file_count;
	unsigned int flag_count;
} Task;

typedef struct {
	Task tasks[MAX_TASK_COUNT];
	unsigned int task_count;
} AllTasks;

typedef struct {
	FILE *workfile;
	AllTasks all_tasks;
	Task selected_task[MAX_TASK_LENGTH];
} WorkContext;

// ========================= GENERAL FUNCTIONS ===========================

void removeTrailingWS(char* str) {
	// Error check
	if (str == NULL || *str == '\0') return;

	// Back of string
    int i = strlen(str) - 1;

    // Iterate backward, finding the last non-whitespace character
    while (i >= 0 && isspace((unsigned char)str[i]))
		i--;
    str[i + 1] = '\0';
}

void cleanAndExit(WorkContext *work_context, int val) {
	if (work_context->workfile != NULL) {
		fclose(work_context->workfile);
	}
	exit(val);
}

// ========================= CLI VALUES ==============================

char flags[NUM_FLAGS][2] = {
	"-h"
};

char* usage[NUM_FLAGS] = {
	"Help"
};

// ========================= CLI FUNCTIONS ===========================

int findFlag(const char* s) {
	int i;
	for (i = 0; i < NUM_FLAGS; i++) {
		if (strcmp(s, flags[i]) == 0) {
			return i;
		} 
	}
	return NOT_FOUND;
}

void printHelp() {
	printf("USAGE: work [TASK] [-FLAG]...\n");
	printf("--------------------------------------\n");
	printf("\t%-15s%-15s", "Flag:", "Description:\n");
	for (int i = 0; i < NUM_FLAGS; i++) {
		printf("\t%15s%15s", flags[i], usage[i]);
	}
	printf("\n--------------------------------------\n");
	printf("Work by Jacob Ramsey.\n");
}

void processCLI(WorkContext *work_context, int argc, char** argv) {
	int flag; bool work_assigned = FALSE;
	for(int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') { // Possible FLAG
			if ((flag = findFlag(argv[i])) == NOT_FOUND) {
				FLAG_ERROR(argv[i]);
				exit(1);
			} else {
				switch(flag) {
					case 0:
						printHelp();
						cleanAndExit(work_context, 0);
				}
			}
		} else { // Possible work specification
			if (work_assigned == FALSE) {
				strcpy(work_context->selected_task->name, argv[i]);
				work_assigned = TRUE;
			} else {
				printf("ERROR: Max 1 work argument.");
				cleanAndExit(work_context, 1);
			}
		}
	}
}

// ========================= WORKFILE FUNCTIONS ===========================

LineState classifyLine(const char* line) {
	// check for comment
	for (int i = 0; line[i] != '\0'; i++) { 
		if (!isspace(line[i])) {
			if (line[i] == '#') {
				return COMMENT;
			} else {
				break;
			}
		}
	}

	// check for task
	if (line[strlen(line) - 1] == ':') 
		return TASK;

	// check for nvim flag
	if (line[0] == '\t' && line[1] == '-') 
		return TASK_FLAG;

	// check for file
	if (line[0] == '\t' && line[1] != '-') 
		return TASK_FILE;
	
	// check for blank line
	if (line[0] == '\0') 
		return BLANK;

	return OTHER;
}

void cleanTask(char* task) {
	task[strlen(task) - 1] = '\0';
}

void processWorkfile(WorkContext *work_context) {
	// Declare vars
	Task current_task = {0};
	char line[MAX_FILE_LENGTH];
	char error_message[MAX_ERROR_LENGTH] = "";
	unsigned int line_count = 1;

	// Open file
	work_context->workfile = fopen("Workfile", "r");

	// Error check
	if (work_context->workfile == NULL) {
		WORKFILE_ERROR("Workfile not found");
		cleanAndExit(work_context, 1);
	}

	// Process each line from workfile
	while (fgets(line, MAX_FILE_LENGTH, work_context->workfile) != NULL) {
		// Read line for task
		removeTrailingWS(line);
		switch(classifyLine(line)) {
			case TASK:
				// Save prevous task
				if (strlen(current_task.name) > 0) { 
					if (work_context->all_tasks.task_count == MAX_TASK_COUNT) {
						snprintf(error_message, MAX_ERROR_LENGTH, "Max number of tasks reached: %u", MAX_TASK_COUNT);
						WORKFILE_ERROR(error_message);
						cleanAndExit(work_context, 1);
					} else if (current_task.file_count == 0){
						WORKFILE_ERROR("Each task must have at least one file.");
						cleanAndExit(work_context, 1);
					}
					work_context->all_tasks.tasks[work_context->all_tasks.task_count++] = current_task;
				}
				// Start new task
				memset(&current_task, 0, sizeof(current_task));
				cleanTask(line);
				strcpy(current_task.name, line);
				break;
			case TASK_FILE:
				// Check for errors
				if (current_task.file_count == MAX_FILE_COUNT) {
					snprintf(error_message, MAX_ERROR_LENGTH, "Max number of task files reached: %u", MAX_FILE_COUNT);
					WORKFILE_ERROR(error_message);
					cleanAndExit(work_context, 1);
				} else if (strlen(current_task.name) == 0) {
					snprintf(error_message, MAX_ERROR_LENGTH, "Task file found on line: %u without parent task.", line_count);
					WORKFILE_ERROR(error_message);
					cleanAndExit(work_context, 1);
				}
				// Add file to task
				strcpy(current_task.files[current_task.file_count++], line);
				break;
			case TASK_FLAG:
				// Check for errors
				if (current_task.flag_count == MAX_FLAG_COUNT) {
					snprintf(error_message, MAX_ERROR_LENGTH, "Max number of task flags reached: %u", MAX_FLAG_COUNT);
					WORKFILE_ERROR(error_message);
					cleanAndExit(work_context, 1);
				} else if (strcmp(current_task.name, "") == 0) {
					snprintf(error_message, MAX_ERROR_LENGTH, "Task flag found on line: %u without parent task.", line_count);
					WORKFILE_ERROR(error_message);
					cleanAndExit(work_context, 1);
				}
				// Add flag to task
				strcpy(current_task.flags[current_task.flag_count++], line);
				break;
			case COMMENT:
			case BLANK:
				break;
			case OTHER:
			default:
				snprintf(error_message, MAX_ERROR_LENGTH, "Unknown line: %u.", line_count);
				WORKFILE_ERROR(error_message);
				cleanAndExit(work_context, 1);
				break;
		}
		line_count++;
	}

	// Save last task found
    if (current_task.name[0] != '\0') {
        if (work_context->all_tasks.task_count >= MAX_TASK_COUNT) {
            snprintf(error_message, MAX_ERROR_LENGTH,"Max number of tasks reached: %u", MAX_TASK_COUNT);
            WORKFILE_ERROR(error_message);
            cleanAndExit(work_context, 1);
        } else if (current_task.file_count == 0) {
			WORKFILE_ERROR("Each task must have at least 1 file.");
			cleanAndExit(work_context, 1);
		}
        work_context->all_tasks.tasks[work_context->all_tasks.task_count++] = current_task;
    }

	// Check if any work at all
	if (work_context->all_tasks.task_count == 0) {
		WORKFILE_ERROR("No work in workfile.");
		exit(1);
	}
}

void executeWork(const WorkContext *work_context) {
	// Declare vars
	unsigned int i; int task_picked = -1;
	char execute[MAX_TASK_LENGTH + (MAX_FLAG_COUNT * 2) + 5] = "nvim ";
	
	// If work specified
	if (work_context->selected_task->name[0] != 0) {
		for (i = 0; i < work_context->all_tasks.task_count; i++) {
			if (strcmp(work_context->all_tasks.tasks[i].name, work_context->selected_task->name) == 0) task_picked = i;
		}
		if (task_picked == -1) {
			WORKFILE_ERROR("Can't find work specified.");
			exit(1);
		}
	} else { // Use default work
		task_picked = 0;
	}
	// Build execute string
	for (i = 0; i < work_context->all_tasks.tasks[task_picked].flag_count; i++) {
		strcat(execute, work_context->all_tasks.tasks[task_picked].flags[i]);
		strcat(execute, " ");
	}
	for (i = 0; i < work_context->all_tasks.tasks[task_picked].file_count; i++) {
		strcat(execute, work_context->all_tasks.tasks[task_picked].files[i]);
		strcat(execute, " ");
	}

	// Execute 
	int execute_response = system(execute);

	// Make sure command executed successfully
	if (execute_response) {
		exit(1);
	}
}

int main(int argc, char** argv) {
	// Declare vars
	WorkContext work_context = {0};
	
	// Process CLI args
	processCLI(&work_context, argc, argv);

	// Process Workfile
	processWorkfile(&work_context);
	
	// Execute work
	executeWork(&work_context);

	// Clean up
	cleanAndExit(&work_context, 0);
	
	return 0;
}
