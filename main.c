/**
 * Jacob Ramsey
 * main.c
 *
 * This is the only file fot the Work program. Thanks for indulging in my relentless over automation.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// MAX VALUES
#define MAX_TASK_LENGTH 100
#define MAX_TASK_COUNT 50
#define MAX_FILE_COUNT 10
#define MAX_FILE_LENGTH 100
#define MAX_FLAG_COUNT 10

// CONSTANTS
#define bool char
#define FALSE 0
#define TRUE 1
#define NOT_FOUND -1
#define NUM_FLAGS 1

// MACROS
#define FLAG_ERROR(x) printf("Uknown Flag: %s\n", x)
#define WORKFILE_ERROR(x) printf("Workfile error: %s\n", x)

// STRUCTS
typedef struct {
	char name[MAX_TASK_LENGTH];
	char files[MAX_FILE_COUNT][MAX_FILE_LENGTH];
	char flags[MAX_FLAG_COUNT][2];
	unsigned int file_count;
	unsigned int flag_count;
} Task;

typedef struct {
	Task task[MAX_TASK_COUNT];
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
	printf("USAGE: work [FILE] [-FLAG]...\n");
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
						exit(0);
				}
			}
		} else { // Possible work specification
			if (work_assigned == FALSE) {
				strcpy(work_context->selected_task->name, argv[i]);
				work_assigned = TRUE;
			} else {
				printf("ERROR: Max 1 work argument.");
				exit(1);
			}
		}
	}
}

// ========================= WORKFILE FUNCTIONS ===========================

bool isComment(const char* line) {
	for (int i = 0; line[i] != '\0'; i++) {
		if (!isspace(line[i])) {
			if (line[i] == '#') {
				return TRUE;
			} else {
				break;
			}
		}
	}
	return FALSE;
}

bool isTask(const char* line) {
	if (line[strlen(line) - 1] == ':') 
		return TRUE;
	return FALSE;
}

bool isFile(const char* line) {
	if (line[0] == '\t' && line[1] != '-') 
		return TRUE;
	return FALSE;
}

bool isNvimFlag(const char* line) {
	if (line[0] == '\t' && line[1] == '-')
		return TRUE;
	return FALSE;
}

void cleanTask(char* task) {
	task[strlen(task) - 1] = '\0';
}

void processWorkfile(WorkContext *work_context) {
	// Declare vars
	char line[100];

	// Open file
	work_context->workfile = fopen("Workfile", "r");

	// Error check
	if (work_context->workfile == NULL) {
		WORKFILE_ERROR("Workfile not found");
		exit(1);
	}

	// Grab each line from workfile
	while (fgets(line, MAX_TASK_LENGTH, work_context->workfile) != NULL) {
		removeTrailingWS(line);
		if (!isComment(line) && isTask(line) && work_context->all_tasks.task_count <= MAX_TASK_COUNT) { /* Found task */
			Task task = {0};
			cleanTask(line);
			strcpy(task.name, line);
			// Grab each file or flag
			while (fgets(line, MAX_FILE_LENGTH, work_context->workfile) != NULL) {
				if (isComment(line)) {																	/* Found comment */
					continue;
				} else if (isFile(line) && task.file_count <= MAX_FILE_COUNT) {							/* Found file for task */
					strcpy(task.files[task.file_count++], line);
				} else if (isNvimFlag(line) && task.flag_count <= MAX_FLAG_COUNT) {						/* Found flag for task */
					strncpy(task.flags[task.flag_count++], line + 1, 2); 
				} else {
					// seek backwards
					fseek(work_context->workfile, -strlen(line), SEEK_CUR);
					break;
				}
			}
			work_context->all_tasks.task[work_context->all_tasks.task_count++] = task;
		}
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
			if (strcmp(work_context->all_tasks.task[i].name, work_context->selected_task->name) == 0) task_picked = i;
		}
		if (task_picked == -1) {
			WORKFILE_ERROR("Can't find work specified.");
			exit(1);
		}
	} else { // Use default work
		task_picked = 0;
	}
	// Build execute string
	for (i = 0; i < work_context->all_tasks.task[task_picked].flag_count; i++) {
		strcat(execute, work_context->all_tasks.task[task_picked].flags[i]);
		strcat(execute, " ");
	}
	for (i = 0; i < work_context->all_tasks.task[task_picked].file_count; i++) {
		strcat(execute, work_context->all_tasks.task[task_picked].files[i]);
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
	fclose(work_context.workfile);
	
	return 0;
}
