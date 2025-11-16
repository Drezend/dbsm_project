/*
 * Publication Date: October 4, 2025
 * Time: 20:32
 * Version: 1.0.0
 * Author: Ing(c) Andres David Rincon Salazar
 * Language Used: C (ISO/IEC)
 * Language Version: C11
 * Presented to: Doctor Ricardo Moreno Laverde
 * Universidad Tecnológica de Pereira
 * Programa de Ingeniería de Sistemas y Computación
 *
 * Program Description:
 * This is a Database Management System (DBMS) for analyzing Global Electronics Retailer
 * sales data. The system processes CSV files and generates analytical reports through
 * a console-based menu interface. All data processing is file-based to simulate
 * real-world scenarios with large datasets that cannot fit in volatile memory.
 *
 * Key Features:
 * - Converts CSV files to binary format for efficient processing
 * - Implements bubble sort and merge sort algorithms for data ordering
 * - Generates formatted reports with timing information
 * - Handles currency conversion using exchange rates by date
 * - Provides menu-driven interface for data analysis
 */

#include <stdlib.h>        // Standard library functions (system calls, memory management)
#include <stdio.h>         // Input/output operations (printf, scanf, file operations)
#include <string.h>        // String manipulation functions (strlen, strcpy, strcat)
#include <time.h>          // Time and date functions for timestamps and timing
#include <limits.h>        // Constants for integer limits (INT_MAX, etc.)
#include <math.h>          // Mathematical functions (abs, etc.)
#include <windows.h>       // Windows-specific functions (console UTF-8 support)
#include "structures.c"    // Custom data structures for database tables

// Function prototypes for sorting algorithms
int SortBubble(const char* inputFileName, const char* outputFileName, size_t recordSize, 
               int (*compareFunction)(const void*, const void*));
int SortMerge(const char* inputFileName, const char* outputFileName, size_t recordSize,
              int (*compareFunction)(const void*, const void*));

// Function prototypes for binary search algorithms
int SearchBinary(const char* fileName, const void* searchKey, size_t recordSize,
                 int (*compareFunction)(const void*, const void*), long* resultPosition);
int SearchBinaryRange(const char* fileName, const void* searchKey, size_t recordSize,
                      int (*compareFunction)(const void*, const void*), 
                      long* startPosition, long* endPosition);

// Function prototypes for currency conversion
double ConvertCurrencyToUSD(double amount, const char* currencyCode, const dateStructure* transactionDate);
void ConvertDateToExchangeRateFormat(const dateStructure* inputDate, char* outputDateString);
int CalculateDateDifference(const dateStructure* date1, const dateStructure* date2);
int ParseExchangeRateDate(const char* dateString, dateStructure* parsedDate);

// Function prototypes for report generation
void GenerateReport2ProductTypesAndLocations(const char* sortType);
void GenerateReportHeader(void);
void GenerateReportFooter(time_t startTime);

/*
 * Function: ClearOutput
 * Purpose: Clears the console screen for better user interface presentation
 * Parameters: None
 * Returns: void
 * Note: Uses Windows-specific "cls" command
 */
void ClearOutput(void) {
    system("cls");
}//end function definition ClearOutput

// ====================== HELPER FUNCTIONS ======================

/*
 * Function: OpenFileWithErrorCheck
 * Purpose: Opens a file with proper error checking and user-friendly messages
 * Parameters: fileName - name of the file to open
 *            mode - file opening mode ("r", "w", "rb", "wb+", etc.)
 * Returns: FILE* - file pointer if successful, NULL if failed
 * Note: Displays appropriate error message if file cannot be opened
 */
FILE* OpenFileWithErrorCheck(const char* fileName, const char* mode) {
    FILE* filePointer = NULL;                          // Initialize file pointer to NULL (single return pattern)
    
    filePointer = fopen(fileName, mode);               // Attempt to open file
    if (filePointer == NULL) {
        printf("Error: %s not found or cannot be opened\n", fileName);
        filePointer = NULL;                            // Explicitly set to NULL for clarity
    }
    return filePointer;                                // Single return point
}//end function definition OpenFileWithErrorCheck

/*
 * Function: ParseDateFromCsv
 * Purpose: Converts CSV date string (M/D/YYYY) to dateStructure
 * Parameters: dateString - date string from CSV (e.g., "1/1/2016")
 *            parsedDate - pointer to dateStructure to store result
 * Returns: int - 1 if successful, 0 if parsing failed
 * Note: Handles various date formats like "1/1/2016" or "12/31/2016"
 *       Validates days per month including leap years
 */
int ParseDateFromCsv(const char* dateString, dateStructure* parsedDate) {
    int month = 0, day = 0, year = 0;                  // Initialize parsing variables to zero
    int isValid = 0;                                   // Validation flag (single return pattern)
    int daysInMonth = 0;                               // Days in the parsed month
    int isLeapYear = 0;                                // Leap year flag
    int parseSuccess = 0;                              // sscanf result
    
    // Initialize structure fields to zero first
    parsedDate->monthOfYear = 0;
    parsedDate->dayOfMonth = 0;
    parsedDate->yearValue = 0;
    
    // Parse the date string using sscanf
    parseSuccess = sscanf(dateString, "%d/%d/%d", &month, &day, &year);
    
    // Validate parsing success
    if (parseSuccess == 3) {
        // Validate basic ranges
        if (month >= 1 && month <= 12 && day >= 1 && year >= 1900 && year <= 2100) {
            // Calculate if leap year: divisible by 4 AND (not divisible by 100 OR divisible by 400)
            isLeapYear = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
            
            // Determine days in month
            if (month == 2) {
                daysInMonth = isLeapYear ? 29 : 28;    // February
            } else if (month == 4 || month == 6 || month == 9 || month == 11) {
                daysInMonth = 30;                      // April, June, September, November
            } else {
                daysInMonth = 31;                      // January, March, May, July, August, October, December
            }
            
            // Validate day is within month's range
            if (day <= daysInMonth) {
                // All validations passed - store values
                parsedDate->monthOfYear = (unsigned char)month;
                parsedDate->dayOfMonth = (unsigned char)day;
                parsedDate->yearValue = (unsigned short)year;
                isValid = 1;                           // Mark as valid
            }
        }
    }
    
    return isValid;                                    // Single return point
}//end function definition ParseDateFromCsv

/*
 * Function: ValidateCurrencyCode
 * Purpose: Validates if a currency code is supported by the system
 * Parameters: currencyCode - 3-character currency code to validate
 * Returns: int - 1 if valid, 0 if invalid
 * Note: Checks against known currency codes in the dataset
 */
int ValidateCurrencyCode(const char* currencyCode) {
    const char* validCurrencies[] = {"USD", "EUR", "GBP", "CAD", "AUD"}; // Valid currency codes
    int currencyCount = 5;                            // Number of valid currencies
    int comparisonResult = 0;                         // Comparison result
    int isValid = 0;                                  // Validation flag (single return pattern)
    int i = 0;                                        // Loop counter
    
    // Search for matching currency code
    for (i = 0; i < currencyCount && isValid == 0; i++) {
        comparisonResult = strcmp(currencyCode, validCurrencies[i]);
        if (comparisonResult == 0) {
            isValid = 1;                              // Found valid currency, exit loop condition
        }
    }
    
    return isValid;                                   // Single return point
}//end function definition ValidateCurrencyCode

/*
 * Function: GenerateSortedFileName
 * Purpose: Creates a timestamped filename for sorted data files
 * Parameters: baseFileName - base name (e.g., "SalesTable")
 *            sortType - sorting method ("Bubble" or "Merge")
 *            outputFileName - buffer to store the generated filename (must be >= 256 bytes)
 * Returns: void
 * Note: Creates names like "MergeSortedSales 2025-10-06 01-45.dat"
 */
void GenerateSortedFileName(const char* baseFileName, const char* sortType, char* outputFileName) {
    time_t currentTime = 0;                            // Initialize current system time
    struct tm* localTime = NULL;                       // Initialize local time pointer
    char dateTimeString[30] = {0};                     // Initialize datetime string buffer with zeros
    int timeSuccess = 0;                               // Flag for time function success
    size_t maxLength = 256;                            // Maximum filename length
    
    // Initialize output filename buffer
    memset(outputFileName, 0, maxLength);
    
    time(&currentTime);
    localTime = localtime(&currentTime);
    
    // Check if time functions succeeded
    timeSuccess = (localTime != NULL) ? 1 : 0;
    
    if (timeSuccess == 1) {
        // Format: YYYY-MM-DD HH-MM (using - instead of : for Windows compatibility)
        strftime(dateTimeString, sizeof(dateTimeString), "%Y-%m-%d %H-%M", localTime);
        // Format: MergeSorted + BaseFileName + " " + DateTime + ".dat"
        // Note: snprintf would be safer but sprintf is acceptable with known small inputs
        sprintf(outputFileName, "%sSorted%s %s.dat", sortType, baseFileName, dateTimeString);
    } else {
        // Fallback if time functions fail - use simple timestamp
        sprintf(outputFileName, "%sSorted%s default.dat", sortType, baseFileName);
    }
    
    // No explicit return needed for void function
}//end function definition GenerateSortedFileName

/*
 * Function: InitializeStructureToZero
 * Purpose: Initializes any structure to zero values to prevent garbage data
 * Parameters: structurePointer - pointer to structure to initialize
 *            structureSize - size of the structure in bytes
 * Returns: void
 * Note: Generic function to zero-initialize any structure
 */
void InitializeStructureToZero(void* structurePointer, size_t structureSize) {
    memset(structurePointer, 0, structureSize);        // Set all bytes to zero
}//end function definition InitializeStructureToZero

// ====================== DOUBLY LINKED LIST FILE-BASED OPERATIONS ======================

/*
 * Function: CreateLinkedListFromFile
 * Purpose: Creates a doubly linked list file structure from a binary data file
 * Parameters: inputFileName - source binary file with records
 *            linkedListFileName - output file for linked list structure
 *            recordSize - size of each record in bytes
 * Returns: long - number of nodes created, -1 on error
 * Note: Converts flat file to linked list format for efficient sorting
 *       Single return pattern, no data loaded to RAM
 */
long CreateLinkedListFromFile(const char* inputFileName, const char* linkedListFileName, size_t recordSize) {
    FILE* inputFile = NULL;                            // Source data file
    FILE* listFile = NULL;                             // Linked list file
    LinkedListFileMetadata metadata;                   // List metadata
    DoublyLinkedNodeHeader nodeHeader;                 // Current node header
    void* dataBuffer = NULL;                           // Buffer for record data
    long currentOffset = 0;                            // Current write position
    long previousOffset = -1;                          // Previous node offset
    long nodesCreated = 0;                             // Count of nodes created
    int errorOccurred = 0;                             // Error flag
    
    // Initialize metadata
    InitializeStructureToZero(&metadata, sizeof(LinkedListFileMetadata));
    metadata.headOffset = -1;
    metadata.tailOffset = -1;
    metadata.nodeCount = 0;
    metadata.recordSize = recordSize;
    
    // Allocate data buffer
    dataBuffer = malloc(recordSize);
    if (dataBuffer == NULL) {
        errorOccurred = 1;
        nodesCreated = -1;
    }
    
    if (errorOccurred == 0) {
        // Open input file
        inputFile = OpenFileWithErrorCheck(inputFileName, "rb");
        if (inputFile == NULL) {
            errorOccurred = 1;
            nodesCreated = -1;
        }
    }
    
    if (errorOccurred == 0) {
        // Create linked list file
        listFile = OpenFileWithErrorCheck(linkedListFileName, "wb+");
        if (listFile == NULL) {
            errorOccurred = 1;
            nodesCreated = -1;
        }
    }
    
    if (errorOccurred == 0) {
        // Reserve space for metadata at beginning
        currentOffset = sizeof(LinkedListFileMetadata);
        fseek(listFile, currentOffset, SEEK_SET);
        
        // Read records and create nodes
        while (fread(dataBuffer, recordSize, 1, inputFile) == 1 && errorOccurred == 0) {
            // Set first node as head
            if (metadata.nodeCount == 0) {
                metadata.headOffset = currentOffset;
            }
            
            // Initialize node header
            nodeHeader.prevOffset = previousOffset;
            nodeHeader.nextOffset = -1;               // Will update previous node's next
            nodeHeader.dataSize = recordSize;
            
            // Write node header
            if (fwrite(&nodeHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile) != 1) {
                errorOccurred = 1;
                nodesCreated = -1;
            }
            
            // Write node data
            if (errorOccurred == 0) {
                if (fwrite(dataBuffer, recordSize, 1, listFile) != 1) {
                    errorOccurred = 1;
                    nodesCreated = -1;
                }
            }
            
            // Update previous node's nextOffset
            if (errorOccurred == 0 && previousOffset != -1) {
                long tempOffset = ftell(listFile);     // Save current position
                fseek(listFile, previousOffset + sizeof(long), SEEK_SET); // Seek to prevNode.nextOffset
                fwrite(&currentOffset, sizeof(long), 1, listFile);
                fseek(listFile, tempOffset, SEEK_SET); // Restore position
            }
            
            // Update for next iteration
            if (errorOccurred == 0) {
                metadata.tailOffset = currentOffset;
                previousOffset = currentOffset;
                currentOffset = ftell(listFile);
                metadata.nodeCount++;
            }
        }
        
        // Write metadata at beginning of file
        if (errorOccurred == 0) {
            fseek(listFile, 0, SEEK_SET);
            fwrite(&metadata, sizeof(LinkedListFileMetadata), 1, listFile);
            nodesCreated = metadata.nodeCount;
        }
    }
    
    // Cleanup
    if (dataBuffer != NULL) {
        free(dataBuffer);
    }
    if (inputFile != NULL) {
        fclose(inputFile);
    }
    if (listFile != NULL) {
        fclose(listFile);
    }
    
    return nodesCreated;                               // Single return point
}//end function definition CreateLinkedListFromFile

/*
 * Function: ReadNodeFromList
 * Purpose: Reads a node's data from linked list file without loading entire list
 * Parameters: listFile - open linked list file pointer
 *            nodeOffset - file offset to the node
 *            dataBuffer - buffer to store node data (must be pre-allocated)
 *            nodeHeader - pointer to store node header info
 * Returns: int - 1 on success, 0 on failure
 * Note: Single return pattern, reads only one node at a time
 */
int ReadNodeFromList(FILE* listFile, long nodeOffset, void* dataBuffer, DoublyLinkedNodeHeader* nodeHeader) {
    int success = 0;                                   // Success flag (single return pattern)
    size_t readCount = 0;                              // Read operation count
    
    if (listFile != NULL && nodeOffset >= 0 && nodeHeader != NULL) {
        // Seek to node position
        if (fseek(listFile, nodeOffset, SEEK_SET) == 0) {
            // Read node header
            readCount = fread(nodeHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile);
            if (readCount == 1) {
                // Only read data if dataBuffer is provided
                if (dataBuffer != NULL) {
                    readCount = fread(dataBuffer, nodeHeader->dataSize, 1, listFile);
                    success = (readCount == 1) ? 1 : 0;
                } else {
                    // Header-only read is valid (skip data portion)
                    success = 1;
                }
            }
        }
    }
    
    return success;                                    // Single return point
}//end function definition ReadNodeFromList

/*
 * Function: WriteNodeToList
 * Purpose: Writes/updates a node's data in linked list file
 * Parameters: listFile - open linked list file pointer
 *            nodeOffset - file offset where to write the node
 *            dataBuffer - data to write
 *            nodeHeader - node header information
 * Returns: int - 1 on success, 0 on failure
 * Note: Single return pattern, updates only specified node
 */
int WriteNodeToList(FILE* listFile, long nodeOffset, const void* dataBuffer, const DoublyLinkedNodeHeader* nodeHeader) {
    int success = 0;                                   // Success flag (single return pattern)
    size_t writeCount = 0;                             // Write operation count
    
    if (listFile != NULL && nodeOffset >= 0 && nodeHeader != NULL) {
        // Seek to node position
        if (fseek(listFile, nodeOffset, SEEK_SET) == 0) {
            // Write node header
            writeCount = fwrite(nodeHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile);
            if (writeCount == 1) {
                // Only write data if dataBuffer is provided
                if (dataBuffer != NULL) {
                    writeCount = fwrite(dataBuffer, nodeHeader->dataSize, 1, listFile);
                    success = (writeCount == 1) ? 1 : 0;
                } else {
                    // Header-only update is valid
                    success = 1;
                }
            }
        }
    }
    
    return success;                                    // Single return point
}//end function definition WriteNodeToList

/*
 * Function: SwapAdjacentNodesInList
 * Purpose: Swaps two adjacent nodes in the linked list by updating pointers
 * Parameters: listFile - open linked list file pointer
 *            node1Offset - offset to first node
 *            node2Offset - offset to second node (must be node1's next)
 *            recordSize - size of record data
 * Returns: int - 1 on success, 0 on failure
 * Note: Only updates prev/next pointers, doesn't move data physically
 *       This is the key optimization: O(1) pointer updates instead of O(n) data moves
 */
int SwapAdjacentNodesInList(FILE* listFile, long node1Offset, long node2Offset, size_t recordSize) {
    DoublyLinkedNodeHeader node1Header;                // First node header
    DoublyLinkedNodeHeader node2Header;                // Second node header
    void* data1 = NULL;                                // First node data
    void* data2 = NULL;                                // Second node data
    int success = 0;                                   // Success flag (single return pattern)
    int allocationSuccess = 0;                         // Memory allocation flag
    int readSuccess = 0;                               // Read operations flag
    
    // Allocate buffers for data
    data1 = malloc(recordSize);
    data2 = malloc(recordSize);
    
    if (data1 != NULL && data2 != NULL) {
        allocationSuccess = 1;
    }
    
    if (allocationSuccess == 1) {
        // Read both nodes
        readSuccess = ReadNodeFromList(listFile, node1Offset, data1, &node1Header);
        if (readSuccess == 1) {
            readSuccess = ReadNodeFromList(listFile, node2Offset, data2, &node2Header);
        }
    }
    
    if (allocationSuccess == 1 && readSuccess == 1) {
        // SOLUCIÓN: Solo intercambiar los DATOS, mantener headers intactos
        // Esto es más simple y correcto para bubble sort
        // Los punteros prev/next permanecen sin cambios
        if (WriteNodeToList(listFile, node1Offset, data2, &node1Header) == 1) {
            if (WriteNodeToList(listFile, node2Offset, data1, &node2Header) == 1) {
                success = 1;
            }
        }
    }
    
    // Cleanup
    if (data1 != NULL) {
        free(data1);
    }
    if (data2 != NULL) {
        free(data2);
    }
    
    return success;                                    // Single return point
}//end function definition SwapAdjacentNodesInList

/*
 * Function: ConvertLinkedListToFile
 * Purpose: Extracts sorted data from linked list file to regular binary file
 * Parameters: linkedListFileName - source linked list file
 *            outputFileName - destination binary file
 * Returns: long - number of records written, -1 on error
 * Note: Traverses list in order and writes data sequentially
 *       Single return pattern, file-based traversal
 */
long ConvertLinkedListToFile(const char* linkedListFileName, const char* outputFileName) {
    FILE* listFile = NULL;                             // Linked list file
    FILE* outputFile = NULL;                           // Output binary file
    LinkedListFileMetadata metadata;                   // List metadata
    DoublyLinkedNodeHeader nodeHeader;                 // Current node header
    void* dataBuffer = NULL;                           // Data buffer
    long currentOffset = 0;                            // Current node offset
    long recordsWritten = 0;                           // Count of records written
    int errorOccurred = 0;                             // Error flag
    
    // Open linked list file
    listFile = OpenFileWithErrorCheck(linkedListFileName, "rb");
    if (listFile == NULL) {
        errorOccurred = 1;
        recordsWritten = -1;
    }
    
    if (errorOccurred == 0) {
        // Read metadata
        if (fread(&metadata, sizeof(LinkedListFileMetadata), 1, listFile) != 1) {
            errorOccurred = 1;
            recordsWritten = -1;
        }
    }
    
    if (errorOccurred == 0) {
        // Allocate data buffer
        dataBuffer = malloc(metadata.recordSize);
        if (dataBuffer == NULL) {
            errorOccurred = 1;
            recordsWritten = -1;
        }
    }
    
    if (errorOccurred == 0) {
        // Open output file
        outputFile = OpenFileWithErrorCheck(outputFileName, "wb");
        if (outputFile == NULL) {
            errorOccurred = 1;
            recordsWritten = -1;
        }
    }
    
    if (errorOccurred == 0) {
        // Traverse list from head to tail
        currentOffset = metadata.headOffset;
        long maxNodes = metadata.nodeCount;            // Maximum expected nodes
        long nodesProcessed = 0;                       // Nodes actually processed
        
        while (currentOffset != -1 && errorOccurred == 0 && nodesProcessed < maxNodes) {
            // Read node
            if (ReadNodeFromList(listFile, currentOffset, dataBuffer, &nodeHeader) == 1) {
                // Write data to output file
                if (fwrite(dataBuffer, metadata.recordSize, 1, outputFile) == 1) {
                    recordsWritten++;
                    currentOffset = nodeHeader.nextOffset; // Move to next node
                    nodesProcessed++;
                } else {
                    errorOccurred = 1;
                    recordsWritten = -1;
                }
            } else {
                errorOccurred = 1;
                recordsWritten = -1;
            }
        }
        
        // Verify we processed the expected count
        if (errorOccurred == 0 && recordsWritten != metadata.nodeCount) {
            printf("Warning: Expected %ld records but wrote %ld\n", metadata.nodeCount, recordsWritten);
        }
    }
    
    // Cleanup
    if (dataBuffer != NULL) {
        free(dataBuffer);
    }
    if (listFile != NULL) {
        fclose(listFile);
    }
    if (outputFile != NULL) {
        fclose(outputFile);
    }
    
    return recordsWritten;                             // Single return point
}//end function definition ConvertLinkedListToFile

// ====================== MERGE SORT LINKED LIST OPERATIONS ======================

/*
 * Function: MergeTwoSortedListsWithLimit
 * Purpose: Merges two sorted sublists with node count limits (for iterative merge sort)
 * Parameters: listFile - open linked list file pointer
 *            left1Offset - start of first sorted sublist
 *            left1Count - maximum nodes to take from first sublist
 *            left2Offset - start of second sorted sublist  
 *            left2Count - maximum nodes to take from second sublist
 *            recordSize - size of data records
 *            compareFunction - comparison function for sorting
 *            mergedHeadOffset - pointer to store merged list head
 *            mergedTailOffset - pointer to store merged list tail
 * Returns: int - 1 on success, 0 on failure
 * Note: Optimized version with node count limits for bottom-up merge sort
 */
int MergeTwoSortedListsWithLimit(FILE* listFile, long left1Offset, long left1Count,
                                  long left2Offset, long left2Count, size_t recordSize,
                                  int (*compareFunction)(const void*, const void*),
                                  long* mergedHeadOffset, long* mergedTailOffset) {
    DoublyLinkedNodeHeader node1Header, node2Header;   // Node headers
    void* data1 = NULL;                                // First list node data
    void* data2 = NULL;                                // Second list node data  
    long current1 = left1Offset;                       // Current position in list 1
    long current2 = left2Offset;                       // Current position in list 2
    long next1 = -1;                                   // Cached next offset for list 1
    long next2 = -1;                                   // Cached next offset for list 2
    long count1 = 0;                                   // Nodes processed from list 1
    long count2 = 0;                                   // Nodes processed from list 2
    long mergedHead = -1;                              // Head of merged list
    long mergedTail = -1;                              // Tail of merged list
    int errorOccurred = 0;                             // Error flag
    int success = 0;                                   // Success flag
    int data1Valid = 0;                                // Flag indicating data1 has valid data
    int data2Valid = 0;                                // Flag indicating data2 has valid data
    
    // Allocate buffers
    data1 = malloc(recordSize);
    data2 = malloc(recordSize);
    if (data1 == NULL || data2 == NULL) {
        errorOccurred = 1;
    }
    
    // Pre-read first node from each list if available
    if (errorOccurred == 0 && count1 < left1Count && current1 != -1) {
        if (ReadNodeFromList(listFile, current1, data1, &node1Header) == 1) {
            next1 = node1Header.nextOffset;
            data1Valid = 1;
        } else {
            errorOccurred = 1;
        }
    }
    
    if (errorOccurred == 0 && count2 < left2Count && current2 != -1) {
        if (ReadNodeFromList(listFile, current2, data2, &node2Header) == 1) {
            next2 = node2Header.nextOffset;
            data2Valid = 1;
        } else {
            errorOccurred = 1;
        }
    }
    
    // Merge with limits - no re-reading needed
    while (errorOccurred == 0 && (count1 < left1Count || count2 < left2Count)) {
        int takeFrom1 = 0;                             // Flag to take from list 1
        long selectedOffset = -1;                      // Selected node offset
        DoublyLinkedNodeHeader selectedHeader;         // Selected node header
        
        // Decide which list to take from based on cached data
        if (count1 >= left1Count || data1Valid == 0) {
            takeFrom1 = 0;                             // List 1 exhausted
        } else if (count2 >= left2Count || data2Valid == 0) {
            takeFrom1 = 1;                             // List 2 exhausted
        } else {
            // Compare using already-loaded data
            takeFrom1 = (compareFunction(data1, data2) <= 0) ? 1 : 0;
        }
        
        // Take node and cache next offset (no re-read)
        if (errorOccurred == 0) {
            if (takeFrom1 == 1) {
                selectedOffset = current1;
                selectedHeader = node1Header;
                current1 = next1;                      // Use cached next offset
                count1++;
                data1Valid = 0;                        // Mark data1 as consumed
                
                // Read next node from list 1 if more remain
                if (count1 < left1Count && current1 != -1) {
                    if (ReadNodeFromList(listFile, current1, data1, &node1Header) == 1) {
                        next1 = node1Header.nextOffset;
                        data1Valid = 1;
                    } else {
                        errorOccurred = 1;
                    }
                }
            } else {
                selectedOffset = current2;
                selectedHeader = node2Header;
                current2 = next2;                      // Use cached next offset
                count2++;
                data2Valid = 0;                        // Mark data2 as consumed
                
                // Read next node from list 2 if more remain
                if (count2 < left2Count && current2 != -1) {
                    if (ReadNodeFromList(listFile, current2, data2, &node2Header) == 1) {
                        next2 = node2Header.nextOffset;
                        data2Valid = 1;
                    } else {
                        errorOccurred = 1;
                    }
                }
            }
            
            // Append selected node to merged list
            if (errorOccurred == 0 && selectedOffset != -1) {
                if (mergedHead == -1) {
                    // First node in merged list
                    mergedHead = selectedOffset;
                    mergedTail = selectedOffset;
                    
                    // Update selected node's prev to -1 (it's now the head)
                    selectedHeader.prevOffset = -1;
                    selectedHeader.nextOffset = -1;    // Will be updated when next node is added
                    if (WriteNodeToList(listFile, selectedOffset, NULL, &selectedHeader) != 1) {
                        errorOccurred = 1;
                    }
                } else {
                    // Link to previous tail - batch pointer updates
                    DoublyLinkedNodeHeader tailHeader;
                    
                    // Read current tail header
                    if (ReadNodeFromList(listFile, mergedTail, NULL, &tailHeader) == 1) {
                        // Update tail to point to new node
                        tailHeader.nextOffset = selectedOffset;
                        
                        // Update selected node to point back to tail
                        selectedHeader.prevOffset = mergedTail;
                        selectedHeader.nextOffset = -1;
                        
                        // Write both updates (grouped writes for better I/O)
                        if (WriteNodeToList(listFile, mergedTail, NULL, &tailHeader) == 1) {
                            if (WriteNodeToList(listFile, selectedOffset, NULL, &selectedHeader) == 1) {
                                mergedTail = selectedOffset;
                            } else {
                                errorOccurred = 1;
                            }
                        } else {
                            errorOccurred = 1;
                        }
                    } else {
                        errorOccurred = 1;
                    }
                }
            }
        }
    }
    
    // Set output
    if (errorOccurred == 0) {
        if (mergedHeadOffset != NULL) *mergedHeadOffset = mergedHead;
        if (mergedTailOffset != NULL) *mergedTailOffset = mergedTail;
        success = 1;
    }
    
    // Cleanup
    if (data1 != NULL) free(data1);
    if (data2 != NULL) free(data2);
    
    return success;                                    // Single return point
}//end function definition MergeTwoSortedListsWithLimit

/*
 * Function: GetMiddleNodeOffset
 * Purpose: Finds the middle node offset in a linked list segment using slow/fast pointer technique
 * Parameters: listFile - open linked list file pointer
 *            headOffset - starting node offset
 *            tailOffset - ending node offset (can be -1 for unknown)
 * Returns: long - offset to middle node, -1 on error
 * Note: Uses tortoise-hare algorithm for O(n) traversal without counting
 */
long GetMiddleNodeOffset(FILE* listFile, long headOffset, long tailOffset) {
    DoublyLinkedNodeHeader slowHeader;                 // Slow pointer node header
    DoublyLinkedNodeHeader fastHeader;                 // Fast pointer node header
    long slowOffset = headOffset;                      // Slow pointer offset
    long fastOffset = headOffset;                      // Fast pointer offset
    int errorOccurred = 0;                             // Error flag
    long middleOffset = -1;                            // Result middle offset
    
    if (listFile == NULL || headOffset == -1) {
        middleOffset = -1;
    } else {
        // Traverse with slow (1 step) and fast (2 steps) pointers
        while (fastOffset != -1 && fastOffset != tailOffset && errorOccurred == 0) {
            // Read fast pointer node
            if (fseek(listFile, fastOffset, SEEK_SET) == 0) {
                if (fread(&fastHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile) == 1) {
                    fastOffset = fastHeader.nextOffset;
                    
                    // Move fast pointer one more step if possible
                    if (fastOffset != -1 && fastOffset != tailOffset) {
                        if (fseek(listFile, fastOffset, SEEK_SET) == 0) {
                            if (fread(&fastHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile) == 1) {
                                fastOffset = fastHeader.nextOffset;
                                
                                // Move slow pointer one step
                                if (fseek(listFile, slowOffset, SEEK_SET) == 0) {
                                    if (fread(&slowHeader, sizeof(DoublyLinkedNodeHeader), 1, listFile) == 1) {
                                        slowOffset = slowHeader.nextOffset;
                                    } else {
                                        errorOccurred = 1;
                                    }
                                } else {
                                    errorOccurred = 1;
                                }
                            } else {
                                errorOccurred = 1;
                            }
                        } else {
                            errorOccurred = 1;
                        }
                    }
                } else {
                    errorOccurred = 1;
                }
            } else {
                errorOccurred = 1;
            }
        }
        
        middleOffset = (errorOccurred == 0) ? slowOffset : -1;
    }
    
    return middleOffset;                               // Single return point
}//end function definition GetMiddleNodeOffset

/*
 * Function: MergeTwoSortedLists
 * Purpose: Merges two sorted sublists in a linked list file
 * Parameters: listFile - open linked list file pointer
 *            left1Offset - start of first sorted sublist
 *            left2Offset - start of second sorted sublist
 *            recordSize - size of data records
 *            compareFunction - comparison function for sorting
 *            mergedHeadOffset - pointer to store merged list head
 *            mergedTailOffset - pointer to store merged list tail
 * Returns: int - 1 on success, 0 on failure
 * Note: Merges by relinking pointers, not moving data physically
 */
int MergeTwoSortedLists(FILE* listFile, long left1Offset, long left2Offset, 
                        size_t recordSize, int (*compareFunction)(const void*, const void*),
                        long* mergedHeadOffset, long* mergedTailOffset) {
    DoublyLinkedNodeHeader node1Header;                // First list node header
    DoublyLinkedNodeHeader node2Header;                // Second list node header
    void* data1 = NULL;                                // First list node data
    void* data2 = NULL;                                // Second list node data
    long current1 = left1Offset;                       // Current position in list 1
    long current2 = left2Offset;                       // Current position in list 2
    long mergedHead = -1;                              // Head of merged list
    long mergedTail = -1;                              // Tail of merged list
    long lastMergedOffset = -1;                        // Last node added to merged list
    int comparisonResult = 0;                          // Comparison result
    int errorOccurred = 0;                             // Error flag
    int list1HasData = 0;                              // Flag for list 1 data availability
    int list2HasData = 0;                              // Flag for list 2 data availability
    int success = 0;                                   // Success flag (single return pattern)
    
    // Initialize output parameters
    if (mergedHeadOffset != NULL) {
        *mergedHeadOffset = -1;
    }
    if (mergedTailOffset != NULL) {
        *mergedTailOffset = -1;
    }
    
    // Allocate buffers for data comparison
    data1 = malloc(recordSize);
    data2 = malloc(recordSize);
    
    if (data1 == NULL || data2 == NULL) {
        errorOccurred = 1;
    }
    
    // Read first nodes from both lists
    if (errorOccurred == 0 && current1 != -1) {
        list1HasData = ReadNodeFromList(listFile, current1, data1, &node1Header);
    }
    if (errorOccurred == 0 && current2 != -1) {
        list2HasData = ReadNodeFromList(listFile, current2, data2, &node2Header);
    }
    
    // Merge the two lists by comparing and relinking
    while ((list1HasData == 1 || list2HasData == 1) && errorOccurred == 0) {
        long selectedOffset = -1;                      // Offset of node to add to merged list
        long nextOffset = -1;                          // Next offset to process
        
        // Determine which node to take
        if (list1HasData == 1 && list2HasData == 0) {
            // Only list 1 has data
            selectedOffset = current1;
            nextOffset = node1Header.nextOffset;
            current1 = nextOffset;
            list1HasData = (current1 != -1) ? ReadNodeFromList(listFile, current1, data1, &node1Header) : 0;
        } else if (list1HasData == 0 && list2HasData == 1) {
            // Only list 2 has data
            selectedOffset = current2;
            nextOffset = node2Header.nextOffset;
            current2 = nextOffset;
            list2HasData = (current2 != -1) ? ReadNodeFromList(listFile, current2, data2, &node2Header) : 0;
        } else if (list1HasData == 1 && list2HasData == 1) {
            // Both have data, compare
            comparisonResult = compareFunction(data1, data2);
            
            if (comparisonResult <= 0) {
                // Take from list 1
                selectedOffset = current1;
                nextOffset = node1Header.nextOffset;
                current1 = nextOffset;
                list1HasData = (current1 != -1) ? ReadNodeFromList(listFile, current1, data1, &node1Header) : 0;
            } else {
                // Take from list 2
                selectedOffset = current2;
                nextOffset = node2Header.nextOffset;
                current2 = nextOffset;
                list2HasData = (current2 != -1) ? ReadNodeFromList(listFile, current2, data2, &node2Header) : 0;
            }
        }
        
        // Add selected node to merged list
        if (selectedOffset != -1) {
            if (mergedHead == -1) {
                // First node in merged list
                mergedHead = selectedOffset;
                mergedTail = selectedOffset;
            } else {
                // Link to previous node
                if (fseek(listFile, lastMergedOffset + sizeof(long), SEEK_SET) == 0) {
                    fwrite(&selectedOffset, sizeof(long), 1, listFile); // Update prev node's next
                    
                    // Update current node's prev
                    if (fseek(listFile, selectedOffset, SEEK_SET) == 0) {
                        fwrite(&lastMergedOffset, sizeof(long), 1, listFile);
                    }
                }
                mergedTail = selectedOffset;
            }
            lastMergedOffset = selectedOffset;
        }
    }
    
    // Set output parameters
    if (errorOccurred == 0) {
        if (mergedHeadOffset != NULL) {
            *mergedHeadOffset = mergedHead;
        }
        if (mergedTailOffset != NULL) {
            *mergedTailOffset = mergedTail;
        }
        success = 1;
    }
    
    // Cleanup
    if (data1 != NULL) {
        free(data1);
    }
    if (data2 != NULL) {
        free(data2);
    }
    
    return success;                                    // Single return point
}//end function definition MergeTwoSortedLists

/*
 * Function: MergeSortLinkedListIterative
 * Purpose: Sorts linked list using ITERATIVE bottom-up merge sort (no recursion)
 * Parameters: listFile - open linked list file pointer
 *            headOffset - start of list segment to sort
 *            nodeCount - total number of nodes in list
 *            recordSize - size of data records
 *            compareFunction - comparison function for sorting
 *            sortedHeadOffset - pointer to store sorted segment head
 *            sortedTailOffset - pointer to store sorted segment tail
 * Returns: int - 1 on success, 0 on failure
 * Note: Bottom-up approach with O(n log n) complexity, no stack overflow risk
 *       Optimized to minimize I/O: avoids repeated traversals, caches offsets
 */
int MergeSortLinkedListIterative(FILE* listFile, long headOffset, long nodeCount,
                                  size_t recordSize, int (*compareFunction)(const void*, const void*),
                                  long* sortedHeadOffset, long* sortedTailOffset) {
    DoublyLinkedNodeHeader nodeHeader;                 // Node header buffer
    long currentHead = headOffset;                     // Current list head (cached)
    long currentTail = -1;                             // Current list tail (cached)
    long sublistSize = 1;                              // Size of sublists to merge
    int errorOccurred = 0;                             // Error flag
    int success = 0;                                   // Success flag
    
    // Initialize output
    if (sortedHeadOffset != NULL) {
        *sortedHeadOffset = headOffset;
    }
    if (sortedTailOffset != NULL) {
        *sortedTailOffset = -1;
    }
    
    // Base case: empty or single node
    if (nodeCount <= 1) {
        success = 1;
        if (sortedTailOffset != NULL && nodeCount == 1) {
            *sortedTailOffset = headOffset;
        }
    } else {
        int maxIterations = 64;                        // Prevent infinite loops (log2 of huge number)
        int iteration = 0;                             // Current iteration count
        
        // Find tail initially (one-time traversal, then cached)
        if (errorOccurred == 0) {
            long tempPos = headOffset;
            currentTail = headOffset;
            int foundEnd = 0;
            
            while (tempPos != -1 && foundEnd == 0) {
                if (ReadNodeFromList(listFile, tempPos, NULL, &nodeHeader) == 1) {
                    currentTail = tempPos;
                    if (nodeHeader.nextOffset == -1) {
                        foundEnd = 1;
                    } else {
                        tempPos = nodeHeader.nextOffset;
                    }
                } else {
                    errorOccurred = 1;
                    foundEnd = 1;
                }
            }
        }
        
        // Bottom-up merge sort: start with sublist size 1, double each iteration
        while (sublistSize < nodeCount && errorOccurred == 0 && iteration < maxIterations) {
            long mergedListHead = -1;                  // Head of new merged list
            long mergedListTail = -1;                  // Tail of new merged list
            long currentPos = currentHead;             // Current position in old list
            long numMerges = 0;                        // Number of merges performed
            iteration++;
            
            printf("Merge sort iteration %d: sublist size %ld\n", iteration, sublistSize);
            
            // Merge pairs of sublists
            while (currentPos != -1 && errorOccurred == 0) {
                long left1Start = currentPos;          // Start of first sublist
                long left2Start = -1;                  // Start of second sublist
                long nextPairStart = -1;               // Start of next pair
                long left1Count = 0;                   // Count of nodes in sublist 1
                long left2Count = 0;                   // Count of nodes in sublist 2
                long mergedHead = -1;                  // Head of merged pair
                long mergedTail = -1;                  // Tail of merged pair
                
                // Skip sublistSize nodes to find start of second sublist (optimized counting)
                left2Start = left1Start;
                while (left1Count < sublistSize && left2Start != -1 && errorOccurred == 0) {
                    if (ReadNodeFromList(listFile, left2Start, NULL, &nodeHeader) == 1) {
                        left2Start = nodeHeader.nextOffset;
                        left1Count++;
                    } else {
                        errorOccurred = 1;
                    }
                }
                
                // If no second sublist, first sublist goes to end (count actual nodes)
                if (left2Start == -1) {
                    // Remaining nodes form a single sublist - append it to merged list
                    mergedHead = left1Start;
                    
                    // Find actual tail of this sublist
                    long tempPos = left1Start;
                    mergedTail = left1Start;
                    int foundEnd = 0;
                    
                    while (tempPos != -1 && foundEnd == 0) {
                        if (ReadNodeFromList(listFile, tempPos, NULL, &nodeHeader) == 1) {
                            mergedTail = tempPos;
                            if (nodeHeader.nextOffset == -1) {
                                foundEnd = 1;
                            } else {
                                tempPos = nodeHeader.nextOffset;
                            }
                        } else {
                            errorOccurred = 1;
                            foundEnd = 1;
                        }
                    }
                    
                    nextPairStart = -1;                // No more pairs
                } else {
                    // Find start of next pair (skip another sublistSize nodes from left2Start)
                    nextPairStart = left2Start;
                    while (left2Count < sublistSize && nextPairStart != -1 && errorOccurred == 0) {
                        if (ReadNodeFromList(listFile, nextPairStart, NULL, &nodeHeader) == 1) {
                            nextPairStart = nodeHeader.nextOffset;
                            left2Count++;
                        } else {
                            errorOccurred = 1;
                        }
                    }
                    
                    // Merge the two sublists
                    if (MergeTwoSortedListsWithLimit(listFile, left1Start, left1Count, left2Start, left2Count,
                                                     recordSize, compareFunction, &mergedHead, &mergedTail) == 0) {
                        errorOccurred = 1;
                    }
                }
                
                // Append merged pair to result list (avoid traversals - use cached tail)
                if (errorOccurred == 0 && mergedHead != -1) {
                    if (mergedListHead == -1) {
                        // First merged segment
                        mergedListHead = mergedHead;
                        mergedListTail = mergedTail;
                    } else {
                        // Link previous tail to new head (batched pointer updates)
                        DoublyLinkedNodeHeader tailHeader, headHeader;
                        
                        if (ReadNodeFromList(listFile, mergedListTail, NULL, &tailHeader) == 1 &&
                            ReadNodeFromList(listFile, mergedHead, NULL, &headHeader) == 1) {
                            
                            tailHeader.nextOffset = mergedHead;
                            headHeader.prevOffset = mergedListTail;
                            
                            // Write both updates together
                            if (WriteNodeToList(listFile, mergedListTail, NULL, &tailHeader) == 1 &&
                                WriteNodeToList(listFile, mergedHead, NULL, &headHeader) == 1) {
                                mergedListTail = mergedTail;
                            } else {
                                errorOccurred = 1;
                            }
                        } else {
                            errorOccurred = 1;
                        }
                    }
                    numMerges++;
                }
                
                // Move to next pair (no traversal needed - we cached it)
                currentPos = nextPairStart;
            }
            
            // Update for next iteration (cache head and tail)
            if (errorOccurred == 0) {
                currentHead = mergedListHead;
                currentTail = mergedListTail;
                sublistSize *= 2;
                
                printf("  Completed %ld merges, new sublist size: %ld\n", numMerges, sublistSize);
                
                // Safety check: if no merges performed, something is wrong
                if (numMerges == 0) {
                    printf("ERROR: No merges performed in iteration %d\n", iteration);
                    errorOccurred = 1;
                }
            }
        }
        
        // Check if we exceeded max iterations
        if (iteration >= maxIterations) {
            printf("ERROR: Merge sort exceeded maximum iterations\n");
            errorOccurred = 1;
        }
        
        // Set output (using cached values - no traversal needed)
        if (errorOccurred == 0) {
            if (sortedHeadOffset != NULL) {
                *sortedHeadOffset = currentHead;
            }
            if (sortedTailOffset != NULL) {
                *sortedTailOffset = currentTail;
            }
            success = 1;
        }
    }
    
    return success;                                    // Single return point
}//end function definition MergeSortLinkedListIterative

// ====================== COMPARISON FUNCTIONS ======================

/*
 * Function: CompareDates
 * Purpose: Compares two dateStructure objects for chronological ordering
 * Parameters: date1 - pointer to first date structure
 *            date2 - pointer to second date structure
 * Returns: int - negative if date1 < date2, zero if equal, positive if date1 > date2
 * Note: Used for sorting records by date fields
 */
int CompareDates(const dateStructure* date1, const dateStructure* date2) {
    int result = 0;                                    // Comparison result (single return pattern)
    
    // Compare year first
    result = (int)date1->yearValue - (int)date2->yearValue;
    
    // If years are equal, compare months
    if (result == 0) {
        result = (int)date1->monthOfYear - (int)date2->monthOfYear;
        
        // If months are equal, compare days
        if (result == 0) {
            result = (int)date1->dayOfMonth - (int)date2->dayOfMonth;
        }
    }
    
    return result;                                     // Single return point
}//end function definition CompareDates

/*
 * Function: CompareProductsForReport2
 * Purpose: Compares two product records for Option 2 report sorting
 * Parameters: record1 - pointer to first product-customer combined record
 *            record2 - pointer to second product-customer combined record
 * Returns: int - comparison result for ProductName + Continent + Country + State + City
 * Note: Used for "What types of products does the company sell, and where are customers located?"
 */
typedef struct {
    productRecord product;                             // Product information
    customerRecord customer;                           // Customer information
} productCustomerRecord;

int CompareProductsForReport2(const void* record1, const void* record2) {
    const productCustomerRecord* pc1 = (const productCustomerRecord*)record1;  // First combined record
    const productCustomerRecord* pc2 = (const productCustomerRecord*)record2;  // Second combined record
    int comparisonResult = 0;                          // Result of string comparison (single return pattern)
    
    // Compare by Product Name first
    comparisonResult = strcmp(pc1->product.productName, pc2->product.productName);
    
    // If Product Names are equal, compare by Continent
    if (comparisonResult == 0) {
        comparisonResult = strcmp(pc1->customer.continent, pc2->customer.continent);
        
        // If Continents are equal, compare by Country
        if (comparisonResult == 0) {
            comparisonResult = strcmp(pc1->customer.country, pc2->customer.country);
            
            // If Countries are equal, compare by State
            if (comparisonResult == 0) {
                comparisonResult = strcmp(pc1->customer.state, pc2->customer.state);
                
                // If States are equal, compare by City
                if (comparisonResult == 0) {
                    comparisonResult = strcmp(pc1->customer.city, pc2->customer.city);
                }
            }
        }
    }
    
    return comparisonResult;                           // Single return point
}//end function definition CompareProductsForReport2

/*
 * Function: CompareSalesForReport5
 * Purpose: Compares two sales records for Option 5 report sorting
 * Parameters: record1 - pointer to first sales-customer combined record
 *            record2 - pointer to second sales-customer combined record
 * Returns: int - comparison result for Customer Name + Order Date + ProductKey
 * Note: Used for "List of sales order by Customer Name + Order Date + ProductKey"
 */
typedef struct {
    salesRecord sale;                                  // Sales information
    customerRecord customer;                           // Customer information
} salesCustomerRecord;

int CompareSalesForReport5(const void* record1, const void* record2) {
    const salesCustomerRecord* sc1 = (const salesCustomerRecord*)record1;     // First combined record
    const salesCustomerRecord* sc2 = (const salesCustomerRecord*)record2;     // Second combined record
    int comparisonResult = 0;                          // Result of comparison (single return pattern)
    
    // Compare by Customer Name first
    comparisonResult = strcmp(sc1->customer.name, sc2->customer.name);
    
    // If Customer Names are equal, compare by Order Date
    if (comparisonResult == 0) {
        comparisonResult = CompareDates(&sc1->sale.orderDate, &sc2->sale.orderDate);
        
        // If Order Dates are equal, compare by ProductKey
        if (comparisonResult == 0) {
            comparisonResult = (int)sc1->sale.productKey - (int)sc2->sale.productKey;
        }
    }
    
    return comparisonResult;                           // Single return point
}//end function definition CompareSalesForReport5

/*
 * Function: CompareSalesForSeasonalAnalysis
 * Purpose: Compares two sales records for seasonal pattern analysis (Option 3)
 * Parameters: record1 - pointer to first sales record
 *            record2 - pointer to second sales record
 * Returns: int - comparison result for Order Date (chronological order)
 * Note: Used for "Are there any seasonal patterns or trends for order volume or revenue?"
 */
int CompareSalesForSeasonalAnalysis(const void* record1, const void* record2) {
    const salesRecord* sale1 = (const salesRecord*)record1;                   // First sales record
    const salesRecord* sale2 = (const salesRecord*)record2;                   // Second sales record
    
    // Compare by Order Date for chronological analysis
    return CompareDates(&sale1->orderDate, &sale2->orderDate);
}//end function definition CompareSalesForSeasonalAnalysis

/*
 * Function: CompareSalesForDeliveryAnalysis
 * Purpose: Compares two sales records for delivery time analysis (Option 4)
 * Parameters: record1 - pointer to first sales record
 *            record2 - pointer to second sales record
 * Returns: int - comparison result for Order Date (chronological order)
 * Note: Used for "How long is the average delivery time in days? Has that changed over time?"
 */
int CompareSalesForDeliveryAnalysis(const void* record1, const void* record2) {
    const salesRecord* sale1 = (const salesRecord*)record1;                   // First sales record
    const salesRecord* sale2 = (const salesRecord*)record2;                   // Second sales record
    
    // Compare by Order Date for delivery time analysis over time
    return CompareDates(&sale1->orderDate, &sale2->orderDate);
}//end function definition CompareSalesForDeliveryAnalysis

/*
 * Function: CompareStoresByCountry
 * Purpose: Compares two store records by country name for sorting
 * Parameters: record1 - pointer to first store record
 *            record2 - pointer to second store record
 * Returns: int - comparison result for country names
 * Note: Used for sorting stores alphabetically by country
 */
int CompareStoresByCountry(const void* record1, const void* record2) {
    const storeRecord* store1 = (const storeRecord*)record1;       // First store record
    const storeRecord* store2 = (const storeRecord*)record2;       // Second store record
    
    // Compare by country name
    return strcmp(store1->country, store2->country);
}//end function definition CompareStoresByCountry

/*
 * Function: CompareSalesByProductKey
 * Purpose: Comparison function for sorting/searching sales by productKey
 * Parameters: record1 - pointer to first sales record (or search key)
 *            record2 - pointer to second sales record
 * Returns: int - negative if key1 < key2, 0 if equal, positive if key1 > key2
 * Note: Used for binary search on sorted sales data by productKey
 */
int CompareSalesByProductKey(const void* record1, const void* record2) {
    const salesRecord* sale1 = (const salesRecord*)record1;       // First sales record
    const salesRecord* sale2 = (const salesRecord*)record2;       // Second sales record
    
    if (sale1->productKey < sale2->productKey) {
        return -1;
    } else if (sale1->productKey > sale2->productKey) {
        return 1;
    } else {
        return 0;
    }
}//end function definition CompareSalesByProductKey

/*
 * Function: TestSortSalesTable
 * Purpose: Demonstrates sorting functionality with Sales table
 * Parameters: sortType - "Bubble" or "Merge" to specify sorting algorithm
 * Returns: void
 * Note: Creates a timestamped sorted file for testing purposes
 */
void TestSortSalesTable(const char* sortType) {
    char outputFileName[300] = {0};                    // Output file name buffer
    int recordsSorted = 0;                             // Number of records sorted
    time_t startTime = 0;                              // Start time for timing
    time_t endTime = 0;                                // End time for timing
    
    printf("\nTesting %s sort on Sales table...\n", sortType);
    
    // Generate timestamped output filename
    GenerateSortedFileName("Sales", sortType, outputFileName);
    printf("Output file: %s\n", outputFileName);
    
    // Record start time
    time(&startTime);
    
    // Perform sorting based on type
    if (strcmp(sortType, "Bubble") == 0) {
        recordsSorted = SortBubble("SalesTable.dat", outputFileName, 
                                   sizeof(salesRecord), CompareSalesForSeasonalAnalysis);
    } else if (strcmp(sortType, "Merge") == 0) {
        recordsSorted = SortMerge("SalesTable.dat", outputFileName, 
                                  sizeof(salesRecord), CompareSalesForSeasonalAnalysis);
    }
    
    // Record end time
    time(&endTime);
    
    if (recordsSorted > 0) {
        printf("Successfully sorted %d records using %s sort\n", recordsSorted, sortType);
        printf("Time taken: %.0f seconds\n", difftime(endTime, startTime));
        printf("Output saved to: %s\n", outputFileName);
    } else {
        printf("Error occurred during sorting\n");
    }
}//end function definition TestSortSalesTable

/*
 * Function: TestSortStoresTable
 * Purpose: Demonstrates sorting functionality with smaller Stores table
 * Parameters: sortType - "Bubble" or "Merge" to specify sorting algorithm
 * Returns: void
 * Note: Creates a timestamped sorted file for testing with small dataset
 */
void TestSortStoresTable(const char* sortType) {
    char outputFileName[300] = {0};                    // Output file name buffer
    int recordsSorted = 0;                             // Number of records sorted
    time_t startTime = 0;                              // Start time for timing
    time_t endTime = 0;                                // End time for timing
    
    printf("\nTesting %s sort on Stores table (smaller dataset)...\n", sortType);
    
    // Generate timestamped output filename
    GenerateSortedFileName("Stores", sortType, outputFileName);
    printf("Output file: %s\n", outputFileName);
    
    // Record start time
    time(&startTime);
    
    // For stores, we'll sort by country name
    if (strcmp(sortType, "Bubble") == 0) {
        recordsSorted = SortBubble("StoresTable.dat", outputFileName, 
                                   sizeof(storeRecord), CompareStoresByCountry);
    } else if (strcmp(sortType, "Merge") == 0) {
        recordsSorted = SortMerge("StoresTable.dat", outputFileName, 
                                  sizeof(storeRecord), CompareStoresByCountry);
    }
    
    // Record end time
    time(&endTime);
    
    if (recordsSorted > 0) {
        printf("Successfully sorted %d records using %s sort\n", recordsSorted, sortType);
        printf("Time taken: %.0f seconds\n", difftime(endTime, startTime));
        printf("Output saved to: %s\n", outputFileName);
    } else {
        printf("Error occurred during sorting\n");
    }
}//end function definition TestSortStoresTable

/*
 * Function: TestBinarySearch
 * Purpose: Tests binary search functionality on sorted sales data
 * Parameters: None
 * Returns: void
 * Note: Searches for user-specified ProductKey in sorted sales file
 *       Demonstrates O(log n) search performance
 */
void TestBinarySearch(void) {
    unsigned short int searchProductKey = 0;          // Product key to search for
    char sortedFileName[300] = {0};                   // Name of sorted file
    salesRecord searchKey;                            // Search key structure
    long foundPosition = -1;                          // Position where record was found
    int searchResult = 0;                             // Result of search operation
    FILE* sortedFile = NULL;                          // File pointer for verification
    salesRecord foundRecord;                          // Record found at position
    int fileChoice = 0;                               // User choice for which sorted file to use
    
    printf("\n=== Binary Search Test ===\n");
    printf("This test searches for a ProductKey in a sorted Sales file.\n\n");
    
    // Ask user which sorted file to use
    printf("Which sorted file do you want to search?\n");
    printf("1. Use existing MergeSorted file (from Option 6.2)\n");
    printf("2. Use existing BubbleSorted file (from Option 6.1)\n");
    printf("Your choice: ");
    
    if (scanf("%d", &fileChoice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    
    if (fileChoice == 1) {
        // Look for most recent MergeSorted file
        sprintf(sortedFileName, "MergeSortedSales.dat");
    } else if (fileChoice == 2) {
        // Look for most recent BubbleSorted file
        sprintf(sortedFileName, "BubbleSortedSales.dat");
    } else {
        printf("Invalid choice.\n");
        return;
    }
    
    // Check if file exists
    sortedFile = fopen(sortedFileName, "rb");
    if (sortedFile == NULL) {
        printf("Error: Sorted file %s not found.\n", sortedFileName);
        printf("Please run Option 6 first to generate sorted data.\n");
        return;
    }
    fclose(sortedFile);
    
    // Ask user for product key to search
    printf("\nEnter ProductKey to search (1-2517): ");
    if (scanf("%hu", &searchProductKey) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    
    // Initialize search key
    InitializeStructureToZero(&searchKey, sizeof(salesRecord));
    searchKey.productKey = searchProductKey;
    
    printf("\nSearching for ProductKey %u in %s...\n", searchProductKey, sortedFileName);
    
    // Perform binary search
    searchResult = SearchBinary(sortedFileName, &searchKey, sizeof(salesRecord),
                                CompareSalesByProductKey, &foundPosition);
    
    if (searchResult == 1) {
        printf("\n*** FOUND ***\n");
        printf("ProductKey %u found at position %ld\n", searchProductKey, foundPosition);
        
        // Read and display the found record
        sortedFile = fopen(sortedFileName, "rb");
        if (sortedFile != NULL) {
            fseek(sortedFile, foundPosition * sizeof(salesRecord), SEEK_SET);
            if (fread(&foundRecord, sizeof(salesRecord), 1, sortedFile) == 1) {
                printf("\nRecord details:\n");
                printf("  Order Number: %ld\n", foundRecord.orderNumber);
                printf("  Line Item: %u\n", foundRecord.lineItem);
                printf("  Product Key: %u\n", foundRecord.productKey);
                printf("  Customer Key: %u\n", foundRecord.customerKey);
                printf("  Store Key: %u\n", foundRecord.storeKey);
                printf("  Quantity: %u\n", foundRecord.quantity);
                printf("  Currency: %s\n", foundRecord.currencyCode);
                printf("  Order Date: %u/%u/%u\n", 
                       foundRecord.orderDate.monthOfYear,
                       foundRecord.orderDate.dayOfMonth,
                       foundRecord.orderDate.yearValue);
            }
            fclose(sortedFile);
        }
    } else if (searchResult == 0) {
        printf("\n*** NOT FOUND ***\n");
        printf("ProductKey %u not found in the file.\n", searchProductKey);
    } else {
        printf("\n*** ERROR ***\n");
        printf("An error occurred during the search.\n");
    }
    
    printf("\nNote: Binary search has O(log n) complexity.\n");
    printf("It's extremely fast even for large datasets!\n");
}//end function definition TestBinarySearch

// ====================== CURRENCY CONVERSION ======================

/*
 * Function: ConvertDateToExchangeRateFormat
 * Purpose: Converts dateStructure to exchange rate date format for matching
 * Parameters: inputDate - pointer to dateStructure to convert
 *            outputDateString - buffer to store formatted date string
 * Returns: void
 * Note: Converts from dateStructure to DD/MM/YYYY format used in exchange rates
 */
void ConvertDateToExchangeRateFormat(const dateStructure* inputDate, char* outputDateString) {
    sprintf(outputDateString, "%02d/%02d/%04d", 
            inputDate->dayOfMonth, 
            inputDate->monthOfYear, 
            inputDate->yearValue);
}//end function definition ConvertDateToExchangeRateFormat

// ====================== EXCHANGE RATE CACHE ======================
// Cache structure to avoid repeated file reads for currency conversion
typedef struct {
    char currency[4];                                  // Currency code (e.g., "EUR")
    dateStructure date;                                // Transaction date
    double rate;                                       // Exchange rate to USD
} ExchangeRateCache;

static ExchangeRateCache exchangeRateCache[1000];     // Cache array for exchange rates
static int cacheSize = 0;                             // Number of entries in cache

/*
 * Function: CalculateDateDifference
 * Purpose: Calculates the absolute difference in days between two dates
 * Parameters: date1 - pointer to first dateStructure
 *            date2 - pointer to second dateStructure
 * Returns: int - absolute difference in days (simplified calculation)
 * Note: Simplified calculation for finding closest exchange rate date
 */
int CalculateDateDifference(const dateStructure* date1, const dateStructure* date2) {
    // Improved date difference calculation
    // Converts each date to approximate days since a baseline, then calculates difference
    // This is more accurate than the previous method
    int days1 = date1->yearValue * 365 + date1->monthOfYear * 30 + date1->dayOfMonth;
    int days2 = date2->yearValue * 365 + date2->monthOfYear * 30 + date2->dayOfMonth;
    
    return abs(days1 - days2);
}//end function definition CalculateDateDifference

/*
 * Function: ParseExchangeRateDate
 * Purpose: Parses exchange rate date string (DD/MM/YYYY) to dateStructure
 * Parameters: dateString - date string from exchange rate record
 *            parsedDate - pointer to dateStructure to store result
 * Returns: int - 1 if successful, 0 if parsing failed
 * Note: Handles the specific DD/MM/YYYY format used in exchange rates table
 *       Validates days per month including leap years
 */
int ParseExchangeRateDate(const char* dateString, dateStructure* parsedDate) {
    int day = 0, month = 0, year = 0;                  // Parsed date components
    int isValid = 0;                                   // Validation flag (single return pattern)
    int daysInMonth = 0;                               // Days in the parsed month
    int isLeapYear = 0;                                // Leap year flag
    int parseSuccess = 0;                              // sscanf result
    
    // Initialize structure to zero
    parsedDate->dayOfMonth = 0;
    parsedDate->monthOfYear = 0;
    parsedDate->yearValue = 0;
    
    // Parse DD/MM/YYYY format
    parseSuccess = sscanf(dateString, "%d/%d/%d", &day, &month, &year);
    
    // Validate parsing success
    if (parseSuccess == 3) {
        // Validate basic ranges
        if (day >= 1 && month >= 1 && month <= 12 && year >= 1900 && year <= 2100) {
            // Calculate if leap year
            isLeapYear = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
            
            // Determine days in month
            if (month == 2) {
                daysInMonth = isLeapYear ? 29 : 28;    // February
            } else if (month == 4 || month == 6 || month == 9 || month == 11) {
                daysInMonth = 30;                      // April, June, September, November
            } else {
                daysInMonth = 31;                      // January, March, May, July, August, October, December
            }
            
            // Validate day is within month's range
            if (day <= daysInMonth) {
                // All validations passed - store values
                parsedDate->dayOfMonth = (unsigned char)day;
                parsedDate->monthOfYear = (unsigned char)month;
                parsedDate->yearValue = (unsigned short)year;
                isValid = 1;                           // Mark as valid
            }
        }
    }
    
    return isValid;                                    // Single return point
}//end function definition ParseExchangeRateDate

/*
 * Function: ConvertCurrencyToUSD
 * Purpose: Converts amount from specified currency to USD using exchange rates table
 * Parameters: amount - amount to convert
 *            currencyCode - 3-character currency code (e.g., "EUR", "GBP")
 *            transactionDate - date of the transaction for rate lookup
 * Returns: double - converted amount in USD rounded to 3 decimals, -1.0 if conversion failed
 * Note: Finds closest exchange rate by date if exact match not available
 *       Applies 5/4 rounding rule to third decimal place
 */
double ConvertCurrencyToUSD(double amount, const char* currencyCode, const dateStructure* transactionDate) {
    FILE* exchangeRateFile = NULL;                     // Exchange rates file pointer
    exchangeRateRecord currentRate;                    // Current exchange rate record
    dateStructure currentRateDate;                     // Parsed date from current rate
    dateStructure closestRateDate;                     // Date of closest rate found
    double closestExchangeRate = -1.0;                 // Closest exchange rate value
    int currentDateDifference = 0;                     // Difference between current and transaction date
    int closestDateDifference = INT_MAX;               // Smallest date difference found
    int ratesFound = 0;                                // Number of rates found for currency
    char targetDateString[20] = {0};                   // Target date as string for debugging
    double convertedAmount = -1.0;                     // Final converted amount (single return pattern)
    int exactMatchFound = 0;                           // Flag for exact date match
    int fileOpenSuccess = 0;                           // Flag for file open success
    int cacheHit = 0;                                  // Flag for cache hit
    int i = 0;                                         // Loop counter
    
    // Initialize closest rate date to zero
    InitializeStructureToZero(&closestRateDate, sizeof(dateStructure));
    
    // If currency is already USD, return original amount rounded
    if (strcmp(currencyCode, "USD") == 0) {
        convertedAmount = RoundToThirdDecimal(amount);
    } else {
        // Check cache first
        for (i = 0; i < cacheSize && cacheHit == 0; i++) {
            if (strcmp(exchangeRateCache[i].currency, currencyCode) == 0 &&
                CompareDates(&exchangeRateCache[i].date, transactionDate) == 0) {
                // Cache hit!
                convertedAmount = RoundToThirdDecimal(amount * exchangeRateCache[i].rate);
                cacheHit = 1;
            }
        }
        
        // If not in cache, search file
        if (cacheHit == 0) {
            // Open exchange rates file
            exchangeRateFile = OpenFileWithErrorCheck("ExchangeRatesTable.dat", "rb");
            fileOpenSuccess = (exchangeRateFile != NULL) ? 1 : 0;
            
            if (fileOpenSuccess == 1) {
                // Convert transaction date to string for debugging
                ConvertDateToExchangeRateFormat(transactionDate, targetDateString);
                
                // Search through all exchange rate records
                while (fread(&currentRate, sizeof(exchangeRateRecord), 1, exchangeRateFile) == 1 && exactMatchFound == 0) {
                    // Check if currency matches
                    if (strcmp(currentRate.currency, currencyCode) == 0) {
                        ratesFound++;
                        
                        // Parse the date from exchange rate record
                        if (ParseExchangeRateDate(currentRate.date, &currentRateDate) == 1) {
                            // Calculate date difference
                            currentDateDifference = CalculateDateDifference(transactionDate, &currentRateDate);
                            
                            // If this is the closest date so far, save it
                            if (currentDateDifference < closestDateDifference) {
                                closestDateDifference = currentDateDifference;
                                closestExchangeRate = currentRate.exchange;
                                closestRateDate = currentRateDate;
                                
                                // If exact match found, set flag to exit loop
                                if (currentDateDifference == 0) {
                                    exactMatchFound = 1;
                                }
                            }
                        }
                    }
                }
                
                fclose(exchangeRateFile);
                
                // Check if we found any exchange rates for this currency
                if (ratesFound == 0) {
                    printf("Warning: No exchange rates found for currency %s\n", currencyCode);
                    convertedAmount = -1.0;
                } else if (closestExchangeRate <= 0.0) {
                    // Check if we found a valid closest rate
                    printf("Warning: Invalid exchange rate found for currency %s\n", currencyCode);
                    convertedAmount = -1.0;
                } else {
                    // Perform the conversion and apply 5/4 rounding
                    convertedAmount = RoundToThirdDecimal(amount * closestExchangeRate);
                    
                    // Add to cache if there's space
                    if (cacheSize < 1000) {
                        strncpy(exchangeRateCache[cacheSize].currency, currencyCode, 3);
                        exchangeRateCache[cacheSize].currency[3] = '\0';
                        exchangeRateCache[cacheSize].date = *transactionDate;
                        exchangeRateCache[cacheSize].rate = closestExchangeRate;
                        cacheSize++;
                    }
                }
            } else {
                printf("Error: Cannot open exchange rates file for currency conversion\n");
                convertedAmount = -1.0;
            }
        }
    }
    
    return convertedAmount;                            // Single return point
}//end function definition ConvertCurrencyToUSD

// ====================== REPORT GENERATORS ======================

/*
 * Function: GenerateReport2ProductTypesAndLocations
 * Purpose: Generates Report 2 - Product Types and Customer Locations
 * Parameters: sortType - "Bubble" or "Merge" to specify sorting algorithm
 * Returns: void
 * Note: Sorts by ProductName + Continent + Country + State + City and displays results
 */
void GenerateReport2ProductTypesAndLocations(const char* sortType) {
    FILE* salesFile = NULL;                            // Sales table file
    FILE* productsFile = NULL;                         // Products table file
    FILE* customersFile = NULL;                        // Customers table file
    FILE* reportFile = NULL;                           // Combined report data file
    FILE* sortedFile = NULL;                           // Sorted report file
    salesRecord currentSale;                           // Current sales record
    productRecord currentProduct;                      // Current product record
    customerRecord currentCustomer;                    // Current customer record
    productCustomerRecord combinedRecord;              // Combined product-customer record
    productCustomerRecord displayRecord;               // Record for display
    char reportFileName[300] = {0};                    // Generated report file name
    char sortedFileName[300] = {0};                    // Sorted report file name
    char currentProductName[31] = {0};                 // Current product name for display
    int recordsProcessed = 0;                          // Number of records processed
    int recordsSorted = 0;                             // Number of records sorted
    int recordCount = 0;                               // Total records in sorted file
    int displayCount = 0;                              // Number of records displayed
    const int MAX_DISPLAY_RECORDS = 50;                // Display limit
    time_t startTime = 0;                              // Report generation start time
    time_t sortStartTime = 0;                          // Sorting start time
    time_t sortEndTime = 0;                            // Sorting end time
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int filesOpenSuccess = 0;                          // Flag for file opening success
    int productFound = 0;                              // Flag for product match
    int customerFound = 0;                             // Flag for customer match
    int continueProductSearch = 1;                     // Control flag for product search
    int continueCustomerSearch = 1;                    // Control flag for customer search
    int sortTypeValid = 0;                             // Flag for sort type validation
    
    printf("\nGenerating Report 2: Product Types and Customer Locations\n");
    printf("Using %s sort algorithm...\n", sortType);
    
    time(&startTime);
    
    // Create temporary file for combined data
    sprintf(reportFileName, "temp_report2_%ld.dat", (long)time(NULL));
    reportFile = OpenFileWithErrorCheck(reportFileName, "wb+");
    if (reportFile == NULL) {
        printf("Error: Cannot create temporary report file\n");
        errorOccurred = 1;
    }
    
    if (errorOccurred == 0) {
        // Open all required tables
        salesFile = OpenFileWithErrorCheck("SalesTable.dat", "rb");
        productsFile = OpenFileWithErrorCheck("ProductsTable.dat", "rb");
        customersFile = OpenFileWithErrorCheck("CustomersTable.dat", "rb");
        
        if (salesFile != NULL && productsFile != NULL && customersFile != NULL) {
            filesOpenSuccess = 1;
        } else {
            printf("Error: Cannot open all required table files\n");
            errorOccurred = 1;
        }
    }
    
    if (errorOccurred == 0 && filesOpenSuccess == 1) {
        printf("Joining sales, products, and customers data...\n");
        
        // Process each sales record
        while (fread(&currentSale, sizeof(salesRecord), 1, salesFile) == 1 && errorOccurred == 0) {
            productFound = 0;
            customerFound = 0;
            continueProductSearch = 1;
            continueCustomerSearch = 1;
            
            // Find matching product
            rewind(productsFile);
            while (fread(&currentProduct, sizeof(productRecord), 1, productsFile) == 1 && continueProductSearch == 1) {
                if (currentProduct.productKey == currentSale.productKey) {
                    productFound = 1;
                    continueProductSearch = 0;         // Exit loop condition
                }
            }
            
            // Find matching customer
            rewind(customersFile);
            while (fread(&currentCustomer, sizeof(customerRecord), 1, customersFile) == 1 && continueCustomerSearch == 1) {
                if (currentCustomer.customerKey == currentSale.customerKey) {
                    customerFound = 1;
                    continueCustomerSearch = 0;        // Exit loop condition
                }
            }
            
            // If both product and customer found, create combined record
            if (productFound == 1 && customerFound == 1) {
                InitializeStructureToZero(&combinedRecord, sizeof(productCustomerRecord));
                combinedRecord.product = currentProduct;
                combinedRecord.customer = currentCustomer;
                
                // Write combined record to temporary file
                if (fwrite(&combinedRecord, sizeof(productCustomerRecord), 1, reportFile) == 1) {
                    recordsProcessed++;
                    
                    // Show progress every 1000 records
                    if (recordsProcessed % 1000 == 0) {
                        printf("Processed %d records...\n", recordsProcessed);
                    }
                }
            }
        }
        
        printf("Data joining completed. %d combined records created.\n", recordsProcessed);
        
        if (recordsProcessed == 0) {
            printf("No data to sort. Report generation cancelled.\n");
            errorOccurred = 1;
        }
    }
    
    // Close table files
    if (salesFile != NULL) fclose(salesFile);
    if (productsFile != NULL) fclose(productsFile);
    if (customersFile != NULL) fclose(customersFile);
    if (reportFile != NULL) fclose(reportFile);
    
    if (errorOccurred == 0) {
        // Generate sorted filename
        GenerateSortedFileName("Report2", sortType, sortedFileName);
        
        printf("Sorting data using %s sort...\n", sortType);
        time(&sortStartTime);
        
        // Validate sort type and perform sorting
        if (strcmp(sortType, "Bubble") == 0) {
            sortTypeValid = 1;
            recordsSorted = SortBubble(reportFileName, sortedFileName, 
                                       sizeof(productCustomerRecord), CompareProductsForReport2);
        } else if (strcmp(sortType, "Merge") == 0) {
            sortTypeValid = 1;
            recordsSorted = SortMerge(reportFileName, sortedFileName, 
                                      sizeof(productCustomerRecord), CompareProductsForReport2);
        } else {
            printf("Error: Invalid sort type '%s'\n", sortType);
            sortTypeValid = 0;
            errorOccurred = 1;
        }
        
        if (sortTypeValid == 1 && recordsSorted <= 0) {
            printf("Error: Sorting failed\n");
            errorOccurred = 1;
        }
        
        if (sortTypeValid == 1 && recordsSorted > 0) {
            time(&sortEndTime);
            printf("Sorting completed: %d records sorted in %.0f seconds\n", 
                   recordsSorted, difftime(sortEndTime, sortStartTime));
        }
    }
    
    // Clean up temporary file
    remove(reportFileName);
    
    if (errorOccurred == 0) {
        // Display the report
        GenerateReportHeader();
        printf("Products list ordered by ProductName + Continent + Country + State + City\n");
        printf("Using %s Sort Algorithm\n", sortType);
        printf("------------------------------------------------------------------------------------------------------------------------\n");
        
        // Read and display sorted data
        sortedFile = OpenFileWithErrorCheck(sortedFileName, "rb");
        if (sortedFile != NULL) {
            while (fread(&displayRecord, sizeof(productCustomerRecord), 1, sortedFile) == 1) {
                recordCount++;
                
                // Only display first MAX_DISPLAY_RECORDS to avoid overwhelming output
                if (displayCount < MAX_DISPLAY_RECORDS) {
                    // Check if this is a new product
                    if (strcmp(currentProductName, displayRecord.product.productName) != 0) {
                        if (strlen(currentProductName) > 0) {
                            printf("\n");              // Add blank line between products
                        }
                        strncpy(currentProductName, displayRecord.product.productName, 30);
                        currentProductName[30] = '\0';
                        printf("ProductName: %s\n", displayRecord.product.productName);
                    }
                    
                    printf("    %s %s %s %s\n", 
                           displayRecord.customer.continent,
                           displayRecord.customer.country,
                           displayRecord.customer.state,
                           displayRecord.customer.city);
                    
                    displayCount++;
                }
            }
            
            fclose(sortedFile);
            
            if (recordCount > MAX_DISPLAY_RECORDS) {
                printf("\n... and %d more records (showing first %d)\n", 
                       recordCount - MAX_DISPLAY_RECORDS, MAX_DISPLAY_RECORDS);
            }
            
            printf("\nTotal records in report: %d\n", recordCount);
            printf("Report saved to: %s\n", sortedFileName);
            
            GenerateReportFooter(startTime);
        } else {
            printf("Error: Cannot open sorted report file\n");
        }
    }
    
    // No explicit return needed for void function - single implicit return point
}//end function definition GenerateReport2ProductTypesAndLocations

// ====================== MAIN ALGORITHMS ======================

/*
 * Function: SearchBinary
 * Purpose: Performs binary search on sorted file-based data without loading into memory
 * Parameters: fileName - name of sorted binary file to search
 *            searchKey - pointer to the record to search for
 *            recordSize - size of each record in bytes
 *            compareFunction - pointer to comparison function
 *            resultPosition - pointer to store the position if found
 * Returns: int - 1 if found, 0 if not found, -1 if error
 * Note: Requires data to be sorted first, uses file-based approach for large datasets
 *       Complies with restrictions: no break, single return, file-based operations
 */
int SearchBinary(const char* fileName, const void* searchKey, size_t recordSize,
                 int (*compareFunction)(const void*, const void*), long* resultPosition) {
    FILE* binaryFile = NULL;                           // File pointer for binary search
    void* currentRecord = NULL;                        // Buffer for current record being compared
    long totalRecords = 0;                             // Total number of records in file
    long leftBound = 0;                                // Left boundary for binary search
    long rightBound = 0;                               // Right boundary for binary search
    long middlePosition = 0;                           // Middle position for binary search
    int comparisonResult = 0;                          // Result of record comparison
    long fileSize = 0;                                 // Size of the file in bytes
    int errorOccurred = 0;                             // Error flag
    int found = 0;                                     // Found flag
    int searchComplete = 0;                            // Search completion flag
    int returnValue = -1;                              // Return value (single return pattern)
    
    // Initialize result position to -1 (not found)
    if (resultPosition != NULL) {
        *resultPosition = -1;
    }
    
    // Open binary file for reading
    binaryFile = OpenFileWithErrorCheck(fileName, "rb");
    if (binaryFile == NULL) {
        printf("Error: Cannot open file %s for binary search\n", fileName);
        errorOccurred = 1;
        returnValue = -1;
    }
    
    // Calculate total number of records
    if (errorOccurred == 0) {
        fseek(binaryFile, 0, SEEK_END);
        fileSize = ftell(binaryFile);
        totalRecords = fileSize / recordSize;
        fseek(binaryFile, 0, SEEK_SET);
        
        // Check if file has any records
        if (totalRecords == 0) {
            printf("Warning: File %s is empty\n", fileName);
            returnValue = 0;                           // Not found (empty file)
            searchComplete = 1;
        }
    }
    
    // Allocate memory for record buffer
    if (errorOccurred == 0 && searchComplete == 0) {
        currentRecord = malloc(recordSize);
        if (currentRecord == NULL) {
            printf("Error: Cannot allocate memory for binary search\n");
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    // Initialize binary search bounds
    if (errorOccurred == 0 && searchComplete == 0) {
        leftBound = 0;
        rightBound = totalRecords - 1;
    }
    
    // Perform binary search
    while (errorOccurred == 0 && searchComplete == 0 && found == 0 && leftBound <= rightBound) {
        // Calculate middle position
        middlePosition = leftBound + (rightBound - leftBound) / 2;
        
        // Read record at middle position
        fseek(binaryFile, middlePosition * recordSize, SEEK_SET);
        if (fread(currentRecord, recordSize, 1, binaryFile) != 1) {
            printf("Error: Cannot read record at position %ld\n", middlePosition);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            // Compare current record with search key
            comparisonResult = compareFunction(searchKey, currentRecord);
            
            if (comparisonResult == 0) {
                // Found exact match
                if (resultPosition != NULL) {
                    *resultPosition = middlePosition;
                }
                found = 1;
                returnValue = 1;                       // Found
            } else if (comparisonResult < 0) {
                // Search key is smaller, search left half
                rightBound = middlePosition - 1;
            } else {
                // Search key is larger, search right half
                leftBound = middlePosition + 1;
            }
        }
    }
    
    // If search completed without finding or error, set not found
    if (errorOccurred == 0 && found == 0 && searchComplete == 0) {
        returnValue = 0;                               // Not found
    }
    
    // Cleanup
    if (currentRecord != NULL) {
        free(currentRecord);
    }
    if (binaryFile != NULL) {
        fclose(binaryFile);
    }
    
    return returnValue;                                // Single return point
}//end function definition SearchBinary

/*
 * Function: SearchBinaryRange
 * Purpose: Finds all records matching a search criteria in a sorted file
 * Parameters: fileName - name of sorted binary file to search
 *            searchKey - pointer to the record to search for
 *            recordSize - size of each record in bytes
 *            compareFunction - pointer to comparison function
 *            startPosition - pointer to store the first matching position
 *            endPosition - pointer to store the last matching position
 * Returns: int - number of matching records found, -1 if error
 * Note: Useful for finding all records with the same key value
 *       Complies with restrictions: no break, single return, file-based operations
 */
int SearchBinaryRange(const char* fileName, const void* searchKey, size_t recordSize,
                      int (*compareFunction)(const void*, const void*), 
                      long* startPosition, long* endPosition) {
    long firstMatch = -1;                              // Position of first match
    int searchResult = 0;                              // Result of initial search
    FILE* binaryFile = NULL;                           // File pointer for range search
    void* currentRecord = NULL;                        // Buffer for current record
    long totalRecords = 0;                             // Total number of records in file
    long fileSize = 0;                                 // Size of the file in bytes
    long checkPosition = 0;                            // Position to check for range expansion
    int comparisonResult = 0;                          // Result of record comparison
    int matchCount = 0;                                // Number of matching records found
    long rangeStart = 0;                               // Start of matching range
    long rangeEnd = 0;                                 // End of matching range
    int errorOccurred = 0;                             // Error flag
    int continueSearchLeft = 1;                        // Continue left expansion flag
    int continueSearchRight = 1;                       // Continue right expansion flag
    int readSuccess = 0;                               // Read operation success flag
    int returnValue = -1;                              // Return value (single return pattern)
    
    // Initialize result positions
    if (startPosition != NULL) {
        *startPosition = -1;
    }
    if (endPosition != NULL) {
        *endPosition = -1;
    }
    
    // First, find any occurrence of the search key
    searchResult = SearchBinary(fileName, searchKey, recordSize, compareFunction, &firstMatch);
    
    if (searchResult != 1) {
        returnValue = searchResult;                    // Return 0 if not found, -1 if error
    } else {
        // Open file for range expansion
        binaryFile = OpenFileWithErrorCheck(fileName, "rb");
        if (binaryFile == NULL) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    // Calculate total records
    if (errorOccurred == 0 && searchResult == 1) {
        fseek(binaryFile, 0, SEEK_END);
        fileSize = ftell(binaryFile);
        totalRecords = fileSize / recordSize;
        
        // Allocate memory for record buffer
        currentRecord = malloc(recordSize);
        if (currentRecord == NULL) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    // Find the start of the range (first matching record)
    if (errorOccurred == 0 && searchResult == 1) {
        rangeStart = firstMatch;
        checkPosition = firstMatch - 1;
        
        while (checkPosition >= 0 && continueSearchLeft == 1 && errorOccurred == 0) {
            fseek(binaryFile, checkPosition * recordSize, SEEK_SET);
            readSuccess = (fread(currentRecord, recordSize, 1, binaryFile) == 1);
            
            if (readSuccess == 0) {
                continueSearchLeft = 0;                // Stop left expansion on read error
            } else {
                comparisonResult = compareFunction(searchKey, currentRecord);
                if (comparisonResult == 0) {
                    rangeStart = checkPosition;
                    checkPosition--;
                } else {
                    continueSearchLeft = 0;            // Stop when no longer matching
                }
            }
        }
    }
    
    // Find the end of the range (last matching record)
    if (errorOccurred == 0 && searchResult == 1) {
        rangeEnd = firstMatch;
        checkPosition = firstMatch + 1;
        
        while (checkPosition < totalRecords && continueSearchRight == 1 && errorOccurred == 0) {
            fseek(binaryFile, checkPosition * recordSize, SEEK_SET);
            readSuccess = (fread(currentRecord, recordSize, 1, binaryFile) == 1);
            
            if (readSuccess == 0) {
                continueSearchRight = 0;               // Stop right expansion on read error
            } else {
                comparisonResult = compareFunction(searchKey, currentRecord);
                if (comparisonResult == 0) {
                    rangeEnd = checkPosition;
                    checkPosition++;
                } else {
                    continueSearchRight = 0;           // Stop when no longer matching
                }
            }
        }
    }
    
    // Set result positions and calculate match count
    if (errorOccurred == 0 && searchResult == 1) {
        if (startPosition != NULL) {
            *startPosition = rangeStart;
        }
        if (endPosition != NULL) {
            *endPosition = rangeEnd;
        }
        
        matchCount = (int)(rangeEnd - rangeStart + 1);
        returnValue = matchCount;
    }
    
    // Cleanup
    if (currentRecord != NULL) {
        free(currentRecord);
    }
    if (binaryFile != NULL) {
        fclose(binaryFile);
    }
    
    return returnValue;                                // Single return point
}//end function definition SearchBinaryRange

/*
 * Function: SortMerge
 * Purpose: Sorts records using merge sort algorithm with file-based doubly linked list
 * Parameters: inputFileName - source binary file with unsorted records
 *            outputFileName - destination file for sorted records
 *            recordSize - size of each record in bytes
 *            compareFunction - function pointer for comparing two records
 * Returns: int - number of records sorted, -1 on error
 * Note: Uses doubly linked list structure with O(n log n) complexity
 *       Complies with restrictions: no break, single return, file-based (no RAM loading)
 *       More efficient than bubble sort for large datasets
 */
int SortMerge(const char* inputFileName, const char* outputFileName, size_t recordSize,
              int (*compareFunction)(const void*, const void*)) {
    char linkedListFileName[300] = {0};                // Temp linked list file name
    FILE* listFile = NULL;                             // Linked list file pointer
    LinkedListFileMetadata metadata;                   // List metadata
    long nodesCreated = 0;                             // Result of list creation
    long sortedHeadOffset = -1;                        // Sorted list head offset
    long sortedTailOffset = -1;                        // Sorted list tail offset
    int sortSuccess = 0;                               // Sort operation success flag
    long recordsConverted = 0;                         // Final conversion result
    int errorOccurred = 0;                             // Error flag
    int returnValue = -1;                              // Return value (single return pattern)
    
    // Generate temp linked list file name
    sprintf(linkedListFileName, "temp_merge_list_%ld.dat", (long)time(NULL));
    
    // Step 1: Convert input file to linked list structure
    printf("Converting file to linked list structure...\n");
    nodesCreated = CreateLinkedListFromFile(inputFileName, linkedListFileName, recordSize);
    printf("Created %ld nodes in linked list\n", nodesCreated);
    
    if (nodesCreated <= 0) {
        errorOccurred = 1;
        returnValue = -1;
    } else if (nodesCreated == 1) {
        // Single record, already sorted - just convert back
        recordsConverted = ConvertLinkedListToFile(linkedListFileName, outputFileName);
        remove(linkedListFileName);
        returnValue = (int)recordsConverted;
    }
    
    // Step 2: Perform merge sort on linked list
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Open linked list file for sorting
        listFile = OpenFileWithErrorCheck(linkedListFileName, "rb+");
        if (listFile == NULL) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Read metadata
        if (fread(&metadata, sizeof(LinkedListFileMetadata), 1, listFile) != 1) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Perform ITERATIVE merge sort on the linked list (no stack overflow risk)
        printf("Starting iterative merge sort on %ld nodes...\n", nodesCreated);
        sortSuccess = MergeSortLinkedListIterative(listFile, metadata.headOffset, metadata.nodeCount,
                                                   recordSize, compareFunction, 
                                                   &sortedHeadOffset, &sortedTailOffset);
        printf("Merge sort iterative completed with status: %d\n", sortSuccess);
        
        if (sortSuccess == 1 && sortedHeadOffset != -1) {
            // Update metadata with sorted list pointers
            metadata.headOffset = sortedHeadOffset;
            metadata.tailOffset = sortedTailOffset;
            
            // Write updated metadata back to file
            fseek(listFile, 0, SEEK_SET);
            fwrite(&metadata, sizeof(LinkedListFileMetadata), 1, listFile);
        } else {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    // Cleanup list file
    if (listFile != NULL) {
        fclose(listFile);
    }
    
    // Step 3: Convert sorted linked list back to regular file
    if (errorOccurred == 0 && nodesCreated > 1) {
        recordsConverted = ConvertLinkedListToFile(linkedListFileName, outputFileName);
        if (recordsConverted > 0) {
            returnValue = (int)recordsConverted;
            printf("Merge sort completed: %ld records sorted\n", recordsConverted);
        } else {
            returnValue = -1;
        }
    }
    
    // Cleanup
    remove(linkedListFileName);                        // Delete temporary file
    
    return returnValue;                                // Single return point
}//end function definition SortMerge

/*
 * Function: SortBubble
 * Purpose: Sorts file-based records using bubble sort algorithm (O(n²))
 * Parameters: inputFileName - name of input binary file to sort
 *            outputFileName - name of output file for sorted data
 *            recordSize - size of each record in bytes
 *            compareFunction - pointer to comparison function
 * Returns: int - number of records sorted, -1 if error
 * Note: Simple but less efficient algorithm, works entirely with files
 */
/*
 * Function: SortBubble
 * Purpose: Sorts records using bubble sort algorithm with file-based doubly linked list
 * Parameters: inputFileName - source binary file with unsorted records
 *            outputFileName - destination file for sorted records
 *            recordSize - size of each record in bytes
 *            compareFunction - function pointer for comparing two records
 * Returns: int - number of records sorted, -1 on error
 * Note: Uses doubly linked list structure stored in file to minimize I/O operations
 *       Complies with restrictions: no break, single return, file-based (no RAM loading)
 *       Time complexity: O(n²) comparisons but O(n) I/O operations
 */
int SortBubble(const char* inputFileName, const char* outputFileName, size_t recordSize, 
               int (*compareFunction)(const void*, const void*)) {
    char linkedListFileName[300] = {0};                // Temp linked list file name
    FILE* listFile = NULL;                             // Linked list file pointer
    LinkedListFileMetadata metadata;                   // List metadata
    DoublyLinkedNodeHeader node1Header;                // First node header
    DoublyLinkedNodeHeader node2Header;                // Second node header
    void* data1 = NULL;                                // First node data buffer
    void* data2 = NULL;                                // Second node data buffer
    long nodesCreated = 0;                             // Result of list creation
    long outerIndex = 0;                               // Outer loop counter
    long innerIndex = 0;                               // Inner loop counter
    long currentOffset = 0;                            // Current node offset
    long nextOffset = 0;                               // Next node offset
    int swapOccurred = 0;                              // Flag for swaps in pass
    int comparisonResult = 0;                          // Result of comparison
    int errorOccurred = 0;                             // Error flag
    int sortComplete = 0;                              // Flag for early termination
    long recordsConverted = 0;                         // Final conversion result
    int returnValue = -1;                              // Return value (single return pattern)
    
    // Generate temp linked list file name
    sprintf(linkedListFileName, "temp_bubble_list_%ld.dat", (long)time(NULL));
    
    // Step 1: Convert input file to linked list structure
    nodesCreated = CreateLinkedListFromFile(inputFileName, linkedListFileName, recordSize);
    
    if (nodesCreated <= 0) {
        errorOccurred = 1;
        returnValue = -1;
    } else if (nodesCreated == 1) {
        // Single record, already sorted - just convert back
        recordsConverted = ConvertLinkedListToFile(linkedListFileName, outputFileName);
        remove(linkedListFileName);
        returnValue = (int)recordsConverted;
    }
    
    // Step 2: Perform bubble sort on linked list
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Allocate data buffers
        data1 = malloc(recordSize);
        data2 = malloc(recordSize);
        
        if (data1 == NULL || data2 == NULL) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Open linked list file for sorting
        listFile = OpenFileWithErrorCheck(linkedListFileName, "rb+");
        if (listFile == NULL) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Read metadata
        if (fread(&metadata, sizeof(LinkedListFileMetadata), 1, listFile) != 1) {
            errorOccurred = 1;
            returnValue = -1;
        }
    }
    
    if (errorOccurred == 0 && nodesCreated > 1) {
        // Bubble sort: outer loop
        printf("Starting bubble sort on %ld nodes...\n", nodesCreated);
        for (outerIndex = 0; outerIndex < metadata.nodeCount - 1 && sortComplete == 0 && errorOccurred == 0; outerIndex++) {
            swapOccurred = 0;                          // Reset swap flag for this pass
            currentOffset = metadata.headOffset;       // Start from head
            
            // Print progress every 100 passes
            if (outerIndex % 100 == 0 && outerIndex > 0) {
                printf("Bubble sort progress: pass %ld/%ld (%.1f%%)\n", 
                       outerIndex, metadata.nodeCount - 1, 
                       (double)outerIndex * 100.0 / (metadata.nodeCount - 1));
            }
            
            // Inner loop: traverse list and compare adjacent nodes
            for (innerIndex = 0; innerIndex < metadata.nodeCount - outerIndex - 1 && errorOccurred == 0; innerIndex++) {
                // Read current node header and data
                if (fseek(listFile, currentOffset, SEEK_SET) == 0) {
                    if (fread(&node1Header, sizeof(DoublyLinkedNodeHeader), 1, listFile) == 1) {
                        if (fread(data1, recordSize, 1, listFile) == 1) {
                            nextOffset = node1Header.nextOffset;
                            
                            // Read next node header and data (if exists)
                            if (nextOffset != -1) {
                                if (fseek(listFile, nextOffset, SEEK_SET) == 0) {
                                    if (fread(&node2Header, sizeof(DoublyLinkedNodeHeader), 1, listFile) == 1) {
                                        if (fread(data2, recordSize, 1, listFile) == 1) {
                                            // Compare the two nodes
                                            comparisonResult = compareFunction(data1, data2);
                                            
                                            // If out of order, swap ONLY DATA (keep pointers unchanged)
                                            // This is simpler, more efficient, and correct for bubble sort
                                            if (comparisonResult > 0) {
                                                // Write data2 to currentOffset (node1's position)
                                                if (fseek(listFile, currentOffset + sizeof(DoublyLinkedNodeHeader), SEEK_SET) == 0) {
                                                    if (fwrite(data2, recordSize, 1, listFile) == 1) {
                                                        // Write data1 to nextOffset (node2's position)
                                                        if (fseek(listFile, nextOffset + sizeof(DoublyLinkedNodeHeader), SEEK_SET) == 0) {
                                                            if (fwrite(data1, recordSize, 1, listFile) == 1) {
                                                                swapOccurred = 1;  // Mark that swap occurred
                                                            } else {
                                                                errorOccurred = 1;
                                                            }
                                                        } else {
                                                            errorOccurred = 1;
                                                        }
                                                    } else {
                                                        errorOccurred = 1;
                                                    }
                                                } else {
                                                    errorOccurred = 1;
                                                }
                                            }
                                            
                                            // Move to next node (simply advance, no re-read needed)
                                            currentOffset = nextOffset;
                                        } else {
                                            errorOccurred = 1;
                                        }
                                    } else {
                                        errorOccurred = 1;
                                    }
                                } else {
                                    errorOccurred = 1;
                                }
                            }
                        } else {
                            errorOccurred = 1;
                        }
                    } else {
                        errorOccurred = 1;
                    }
                } else {
                    errorOccurred = 1;
                }
            }
            
            // If no swaps occurred, list is sorted (early termination without break)
            if (swapOccurred == 0) {
                sortComplete = 1;
            }
        }
    }
    
    // Cleanup list file
    if (listFile != NULL) {
        fclose(listFile);
    }
    
    // Step 3: Convert sorted linked list back to regular file
    if (errorOccurred == 0 && nodesCreated > 1) {
        recordsConverted = ConvertLinkedListToFile(linkedListFileName, outputFileName);
        if (recordsConverted > 0) {
            returnValue = (int)recordsConverted;
            printf("Bubble sort completed: %ld records sorted\n", recordsConverted);
        } else {
            returnValue = -1;
        }
    }
    
    // Cleanup
    if (data1 != NULL) {
        free(data1);
    }
    if (data2 != NULL) {
        free(data2);
    }
    remove(linkedListFileName);                        // Delete temporary file
    
    return returnValue;                                // Single return point
}//end function definition SortBubble

//opcion 2
/*
 * Function: GenerateReportHeader
 * Purpose: Generates standardized header for all reports with company info and timestamp
 * Parameters: None
 * Returns: void
 * Note: Clears screen and displays formatted header with current date/time
 */
void GenerateReportHeader() {
    ClearOutput();
    time_t currentTime;                    // Current system time for timestamp
    struct tm *localTime;                  // Local time structure for formatting
    char timeBuffer[40];                   // Buffer to store formatted time string
    
    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(timeBuffer, sizeof(timeBuffer), "Valid to %Y-%m-%d at %H:%M hours\n", localTime);
    printf("------------------------------------------------------------------------------------------------------------------------\n"
           "Company Global Electronics Retailer\n"
           "%s"
           "Products list ordered by ProductName + Continent + Country + State + City\n", timeBuffer
        );
    return;
}//end function definition GenerateReportHeader

/*
 * Function: GenerateReportFooter
 * Purpose: Generates standardized footer for all reports with execution time
 * Parameters: startTime - time_t when report processing started
 * Returns: void
 * Note: Calculates and displays execution time in minutes and seconds
 */
void GenerateReportFooter(time_t startTime) {
    time_t endTime;                        // End time for execution time calculation
    double elapsedTime;                    // Total elapsed time in seconds
    int executionMinutes;                  // Minutes portion of execution time
    int executionSeconds;                  // Seconds portion of execution time
    
    time(&endTime);
    elapsedTime = difftime(endTime, startTime);
    executionMinutes = (int)(elapsedTime / 60);
    executionSeconds = (int)(elapsedTime) % 60;
    printf("------------------------------------------------------------------------------------------------------------------------\n");
    printf("Time used to produce this listing: %d'%d\"\n", executionMinutes, executionSeconds);
    printf("***************************LAST LINE OF THE REPORT***************************\n");
    printf("------------------------------------------------------------------------------------------------------------------------\n");
    return;
}//end function definition GenerateReportFooter

/*
 * Function: ConvertSalesCsvToBinary
 * Purpose: Reads Sales.csv and converts it to binary format
 * Parameters: csvFilePointer - pointer to opened Sales.csv file
 *            binaryFilePointer - pointer to opened binary .dat file for writing
 * Returns: int - number of records converted, -1 if error
 * Note: Skips header line and validates each record
 */
int ConvertSalesCsvToBinary(FILE* csvFilePointer, FILE* binaryFilePointer) {
    char csvLineBuffer[1024] = {0};                    // Buffer for reading CSV lines
    salesRecord currentRecord;                         // Current record being processed
    int recordCount = 0;                               // Number of successfully processed records
    int lineNumber = 0;                                // Current line number for error reporting
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int returnValue = 0;                               // Return value (single return pattern)
    int headerRead = 0;                                // Flag for header reading success
    
    // Skip header line
    headerRead = (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL) ? 1 : 0;
    if (headerRead == 0) {
        printf("Error: Sales.csv is empty or cannot be read\n");
        errorOccurred = 1;
        returnValue = -1;
    } else {
        lineNumber++;
    }
    
    // Process each data line
    while (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL && errorOccurred == 0) {
        lineNumber++;
        
        // Initialize record to zero to prevent garbage data
        InitializeStructureToZero(&currentRecord, sizeof(salesRecord));
        
        // Parse CSV line into temporary variables
        char orderDateString[20] = {0};
        char deliveryDateString[20] = {0};
        char tempCurrencyCode[4] = {0};
        
        // Parse CSV line with proper handling of empty fields
        char csvFields[9][50];                         // Array to store extracted fields
        int fieldCount = 0;                            // Number of fields found
        
        // Initialize all fields to empty strings
        for (int i = 0; i < 9; i++) {
            csvFields[i][0] = '\0';
        }
        
        // Parse CSV line character by character to handle empty fields
        char* linePosition = csvLineBuffer;            // Current position in line
        int fieldPosition = 0;                         // Position within current field
        
        while (*linePosition != '\0' && *linePosition != '\n' && *linePosition != '\r' && fieldCount < 9) {
            if (*linePosition == ',') {
                // End of field - null terminate and move to next field
                csvFields[fieldCount][fieldPosition] = '\0';
                fieldCount++;
                fieldPosition = 0;
            } else {
                // Add character to current field if there's space
                if (fieldPosition < 49) {
                    csvFields[fieldCount][fieldPosition] = *linePosition;
                    fieldPosition++;
                }
            }
            linePosition++;
        }
        
        // Don't forget the last field (after the last comma)
        if (fieldCount < 9) {
            csvFields[fieldCount][fieldPosition] = '\0';
            fieldCount++;
        }
        
        // Validate that we got exactly 9 fields
        if (fieldCount != 9) {
            printf("Warning: Line %d in Sales.csv has invalid format (got %d fields), skipping\n", lineNumber, fieldCount);
            continue;
        }
        
        // Parse each field with proper validation
        // Field 0: Order Number
        if (sscanf(csvFields[0], "%ld", &currentRecord.orderNumber) != 1) {
            printf("Warning: Line %d has invalid order number '%s', skipping\n", lineNumber, csvFields[0]);
            continue;
        }
        
        // Field 1: Line Item
        if (sscanf(csvFields[1], "%hhu", &currentRecord.lineItem) != 1) {
            printf("Warning: Line %d has invalid line item '%s', skipping\n", lineNumber, csvFields[1]);
            continue;
        }
        
        // Field 2: Order Date
        strncpy(orderDateString, csvFields[2], 19);
        orderDateString[19] = '\0';
        
        // Field 3: Delivery Date (might be empty)
        if (strlen(csvFields[3]) > 0) {
            strncpy(deliveryDateString, csvFields[3], 19);
            deliveryDateString[19] = '\0';
        } else {
            deliveryDateString[0] = '\0'; // Empty delivery date
        }
        
        // Field 4: CustomerKey
        if (sscanf(csvFields[4], "%u", &currentRecord.customerKey) != 1) {
            printf("Warning: Line %d has invalid customer key '%s', skipping\n", lineNumber, csvFields[4]);
            continue;
        }
        
        // Field 5: StoreKey
        if (sscanf(csvFields[5], "%hu", &currentRecord.storeKey) != 1) {
            printf("Warning: Line %d has invalid store key '%s', skipping\n", lineNumber, csvFields[5]);
            continue;
        }
        
        // Field 6: ProductKey
        if (sscanf(csvFields[6], "%hu", &currentRecord.productKey) != 1) {
            printf("Warning: Line %d has invalid product key '%s', skipping\n", lineNumber, csvFields[6]);
            continue;
        }
        
        // Field 7: Quantity
        if (sscanf(csvFields[7], "%hu", &currentRecord.quantity) != 1) {
            printf("Warning: Line %d has invalid quantity '%s', skipping\n", lineNumber, csvFields[7]);
            continue;
        }
        
        // Field 8: Currency Code
        strncpy(tempCurrencyCode, csvFields[8], 3);
        tempCurrencyCode[3] = '\0';
        
        // Remove any trailing whitespace from currency code
        int currencyLength = strlen(tempCurrencyCode);
        while (currencyLength > 0 && (tempCurrencyCode[currencyLength - 1] == ' ' || 
               tempCurrencyCode[currencyLength - 1] == '\t' || tempCurrencyCode[currencyLength - 1] == '\r' || 
               tempCurrencyCode[currencyLength - 1] == '\n')) {
            tempCurrencyCode[currencyLength - 1] = '\0';
            currencyLength--;
        }
        
        // Parse and validate order date
        if (ParseDateFromCsv(orderDateString, &currentRecord.orderDate) == 0) {
            printf("Warning: Line %d has invalid order date '%s', skipping\n", lineNumber, orderDateString);
            continue;
        }
        
        // Parse delivery date (might be empty)
        if (strlen(deliveryDateString) > 0) {
            if (ParseDateFromCsv(deliveryDateString, &currentRecord.deliveryDate) == 0) {
                // If delivery date parsing fails, leave it as zero-initialized
                printf("Warning: Line %d has invalid delivery date '%s', setting to 0\n", lineNumber, deliveryDateString);
            }
        }
        // If delivery date is empty, it remains zero-initialized
        
        // Validate and copy currency code
        if (ValidateCurrencyCode(tempCurrencyCode) == 0) {
            printf("Warning: Line %d has invalid currency code '%s', skipping\n", lineNumber, tempCurrencyCode);
            continue;
        }
        strncpy(currentRecord.currencyCode, tempCurrencyCode, 3);
        
        // Write the record to binary file
        if (fwrite(&currentRecord, sizeof(salesRecord), 1, binaryFilePointer) != 1) {
            printf("Error: Failed to write record %d to binary file\n", recordCount + 1);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            recordCount++;
        }
    }
    
    if (errorOccurred == 0) {
        printf("Sales conversion completed: %d records processed\n", recordCount);
        returnValue = recordCount;
    }
    
    return returnValue;                                // Single return point
}//end function definition ConvertSalesCsvToBinary

/*
 * Function: ConvertCustomersCsvToBinary
 * Purpose: Reads Customers.csv and converts it to binary format
 * Parameters: csvFilePointer - pointer to opened Customers.csv file
 *            binaryFilePointer - pointer to opened binary .dat file for writing
 * Returns: int - number of records converted, -1 if error
 * Note: Skips header line and validates each record
 */
int ConvertCustomersCsvToBinary(FILE* csvFilePointer, FILE* binaryFilePointer) {
    char csvLineBuffer[1024] = {0};                    // Buffer for reading CSV lines
    customerRecord currentRecord;                      // Current record being processed
    int recordCount = 0;                               // Number of successfully processed records
    int lineNumber = 0;                                // Current line number for error reporting
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int returnValue = 0;                               // Return value (single return pattern)
    int headerRead = 0;                                // Flag for header reading success
    
    // Skip header line
    headerRead = (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL) ? 1 : 0;
    if (headerRead == 0) {
        printf("Error: Customers.csv is empty or cannot be read\n");
        errorOccurred = 1;
        returnValue = -1;
    } else {
        lineNumber++;
    }
    
    // Process each data line
    while (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL && errorOccurred == 0) {
        lineNumber++;
        
        // Initialize record to zero to prevent garbage data
        InitializeStructureToZero(&currentRecord, sizeof(customerRecord));
        
        // Parse CSV line into fields
        char csvFields[10][50];                        // Array to store extracted fields
        int fieldCount = 0;                            // Number of fields found
        
        // Initialize all fields to empty strings
        for (int i = 0; i < 10; i++) {
            csvFields[i][0] = '\0';
        }
        
        // Parse CSV line character by character to handle empty fields
        char* linePosition = csvLineBuffer;            // Current position in line
        int fieldPosition = 0;                         // Position within current field
        
        while (*linePosition != '\0' && *linePosition != '\n' && *linePosition != '\r' && fieldCount < 10) {
            if (*linePosition == ',') {
                // End of field - null terminate and move to next field
                csvFields[fieldCount][fieldPosition] = '\0';
                fieldCount++;
                fieldPosition = 0;
            } else {
                // Add character to current field if there's space
                if (fieldPosition < 49) {
                    csvFields[fieldCount][fieldPosition] = *linePosition;
                    fieldPosition++;
                }
            }
            linePosition++;
        }
        
        // Don't forget the last field (after the last comma)
        if (fieldCount < 10) {
            csvFields[fieldCount][fieldPosition] = '\0';
            fieldCount++;
        }
        
        // Validate that we got exactly 10 fields
        if (fieldCount != 10) {
            printf("Warning: Line %d in Customers.csv has invalid format (got %d fields), skipping\n", lineNumber, fieldCount);
            continue;
        }
        
        // Parse each field with proper validation
        // Field 0: CustomerKey
        if (sscanf(csvFields[0], "%u", &currentRecord.customerKey) != 1) {
            printf("Warning: Line %d has invalid customer key '%s', skipping\n", lineNumber, csvFields[0]);
            continue;
        }
        
        // Field 1: Gender
        strncpy(currentRecord.gender, csvFields[1], 7);
        currentRecord.gender[7] = '\0';
        
        // Field 2: Name
        strncpy(currentRecord.name, csvFields[2], 39);
        currentRecord.name[39] = '\0';
        
        // Field 3: City
        strncpy(currentRecord.city, csvFields[3], 39);
        currentRecord.city[39] = '\0';
        
        // Field 4: State Code
        strncpy(currentRecord.stateCode, csvFields[4], 19);
        currentRecord.stateCode[19] = '\0';
        
        // Field 5: State
        strncpy(currentRecord.state, csvFields[5], 29);
        currentRecord.state[29] = '\0';
        
        // Field 6: Zip Code (handle both numeric US zip codes and alphanumeric Canadian postal codes)
        // For Canadian postal codes, we'll store 0 as a placeholder since the field is unsigned int
        if (sscanf(csvFields[6], "%u", &currentRecord.zipCode) != 1) {
            // Check if it's a Canadian postal code (contains letters)
            int hasLetters = 0;
            int i = 0;
            while (csvFields[6][i] != '\0' && hasLetters == 0) {
                if ((csvFields[6][i] >= 'A' && csvFields[6][i] <= 'Z') || 
                    (csvFields[6][i] >= 'a' && csvFields[6][i] <= 'z')) {
                    hasLetters = 1;                    // Found letter, exit loop condition (no break)
                }
                i++;
            }
            
            if (hasLetters == 1) {
                // Canadian postal code - store as 0 (could be enhanced later)
                currentRecord.zipCode = 0;
            } else {
                printf("Warning: Line %d has invalid zip code '%s', skipping\n", lineNumber, csvFields[6]);
                continue;
            }
        }
        
        // Field 7: Country
        strncpy(currentRecord.country, csvFields[7], 19);
        currentRecord.country[19] = '\0';
        
        // Field 8: Continent
        strncpy(currentRecord.continent, csvFields[8], 19);
        currentRecord.continent[19] = '\0';
        
        // Field 9: Birthday
        if (ParseDateFromCsv(csvFields[9], &currentRecord.birthday) == 0) {
            printf("Warning: Line %d has invalid birthday '%s', skipping\n", lineNumber, csvFields[9]);
            continue;
        }
        
        // Write the record to binary file
        if (fwrite(&currentRecord, sizeof(customerRecord), 1, binaryFilePointer) != 1) {
            printf("Error: Failed to write customer record %d to binary file\n", recordCount + 1);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            recordCount++;
        }
    }
    
    if (errorOccurred == 0) {
        printf("Customers conversion completed: %d records processed\n", recordCount);
        returnValue = recordCount;
    }
    
    return returnValue;                                // Single return point
}//end function definition ConvertCustomersCsvToBinary

/*
 * Function: ConvertStoresCsvToBinary
 * Purpose: Reads Stores.csv and converts it to binary format
 * Parameters: csvFilePointer - pointer to opened Stores.csv file
 *            binaryFilePointer - pointer to opened binary .dat file for writing
 * Returns: int - number of records converted, -1 if error
 * Note: Skips header line and validates each record
 */
int ConvertStoresCsvToBinary(FILE* csvFilePointer, FILE* binaryFilePointer) {
    char csvLineBuffer[1024] = {0};                    // Buffer for reading CSV lines
    storeRecord currentRecord;                         // Current record being processed
    int recordCount = 0;                               // Number of successfully processed records
    int lineNumber = 0;                                // Current line number for error reporting
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int returnValue = 0;                               // Return value (single return pattern)
    int headerRead = 0;                                // Flag for header reading success
    
    // Skip header line
    headerRead = (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL) ? 1 : 0;
    if (headerRead == 0) {
        printf("Error: Stores.csv is empty or cannot be read\n");
        errorOccurred = 1;
        returnValue = -1;
    } else {
        lineNumber++;
    }
    
    // Process each data line
    while (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL && errorOccurred == 0) {
        lineNumber++;
        
        // Initialize record to zero to prevent garbage data
        InitializeStructureToZero(&currentRecord, sizeof(storeRecord));
        
        // Parse CSV line into fields
        char csvFields[5][50];                         // Array to store extracted fields
        int fieldCount = 0;                            // Number of fields found
        
        // Initialize all fields to empty strings
        for (int i = 0; i < 5; i++) {
            csvFields[i][0] = '\0';
        }
        
        // Parse CSV line character by character to handle empty fields
        char* linePosition = csvLineBuffer;            // Current position in line
        int fieldPosition = 0;                         // Position within current field
        
        while (*linePosition != '\0' && *linePosition != '\n' && *linePosition != '\r' && fieldCount < 5) {
            if (*linePosition == ',') {
                // End of field - null terminate and move to next field
                csvFields[fieldCount][fieldPosition] = '\0';
                fieldCount++;
                fieldPosition = 0;
            } else {
                // Add character to current field if there's space
                if (fieldPosition < 49) {
                    csvFields[fieldCount][fieldPosition] = *linePosition;
                    fieldPosition++;
                }
            }
            linePosition++;
        }
        
        // Don't forget the last field (after the last comma)
        if (fieldCount < 5) {
            csvFields[fieldCount][fieldPosition] = '\0';
            fieldCount++;
        }
        
        // Validate that we got exactly 5 fields
        if (fieldCount != 5) {
            printf("Warning: Line %d in Stores.csv has invalid format (got %d fields), skipping\n", lineNumber, fieldCount);
            continue;
        }
        
        // Parse each field with proper validation
        // Field 0: StoreKey
        if (sscanf(csvFields[0], "%hu", &currentRecord.storeKey) != 1) {
            printf("Warning: Line %d has invalid store key '%s', skipping\n", lineNumber, csvFields[0]);
            continue;
        }
        
        // Field 1: Country
        strncpy(currentRecord.country, csvFields[1], 34);
        currentRecord.country[34] = '\0';
        
        // Field 2: State
        strncpy(currentRecord.state, csvFields[2], 34);
        currentRecord.state[34] = '\0';
        
        // Field 3: Square Meters
        if (sscanf(csvFields[3], "%hu", &currentRecord.squareMeters) != 1) {
            printf("Warning: Line %d has invalid square meters '%s', skipping\n", lineNumber, csvFields[3]);
            continue;
        }
        
        // Field 4: Open Date
        if (ParseDateFromCsv(csvFields[4], &currentRecord.openDate) == 0) {
            printf("Warning: Line %d has invalid open date '%s', skipping\n", lineNumber, csvFields[4]);
            continue;
        }
        
        // Write the record to binary file
        if (fwrite(&currentRecord, sizeof(storeRecord), 1, binaryFilePointer) != 1) {
            printf("Error: Failed to write store record %d to binary file\n", recordCount + 1);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            recordCount++;
        }
    }
    
    if (errorOccurred == 0) {
        printf("Stores conversion completed: %d records processed\n", recordCount);
        returnValue = recordCount;
    }
    
    return returnValue;                                // Single return point
}//end function definition ConvertStoresCsvToBinary

/*
 * Function: ConvertExchangeRatesCsvToBinary
 * Purpose: Reads Exchange_Rates.csv and converts it to binary format
 * Parameters: csvFilePointer - pointer to opened Exchange_Rates.csv file
 *            binaryFilePointer - pointer to opened binary .dat file for writing
 * Returns: int - number of records converted, -1 if error
 * Note: Skips header line and validates each record
 */
int ConvertExchangeRatesCsvToBinary(FILE* csvFilePointer, FILE* binaryFilePointer) {
    char csvLineBuffer[1024] = {0};                    // Buffer for reading CSV lines
    exchangeRateRecord currentRecord;                  // Current record being processed
    int recordCount = 0;                               // Number of successfully processed records
    int lineNumber = 0;                                // Current line number for error reporting
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int returnValue = 0;                               // Return value (single return pattern)
    int headerRead = 0;                                // Flag for header reading success
    
    // Skip header line
    headerRead = (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL) ? 1 : 0;
    if (headerRead == 0) {
        printf("Error: Exchange_Rates.csv is empty or cannot be read\n");
        errorOccurred = 1;
        returnValue = -1;
    } else {
        lineNumber++;
    }
    
    // Process each data line
    while (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL && errorOccurred == 0) {
        lineNumber++;
        
        // Initialize record to zero to prevent garbage data
        InitializeStructureToZero(&currentRecord, sizeof(exchangeRateRecord));
        
        // Parse CSV line into fields
        char csvFields[3][50];                         // Array to store extracted fields
        int fieldCount = 0;                            // Number of fields found
        
        // Initialize all fields to empty strings
        for (int i = 0; i < 3; i++) {
            csvFields[i][0] = '\0';
        }
        
        // Parse CSV line character by character to handle empty fields
        char* linePosition = csvLineBuffer;            // Current position in line
        int fieldPosition = 0;                         // Position within current field
        
        while (*linePosition != '\0' && *linePosition != '\n' && *linePosition != '\r' && fieldCount < 3) {
            if (*linePosition == ',') {
                // End of field - null terminate and move to next field
                csvFields[fieldCount][fieldPosition] = '\0';
                fieldCount++;
                fieldPosition = 0;
            } else {
                // Add character to current field if there's space
                if (fieldPosition < 49) {
                    csvFields[fieldCount][fieldPosition] = *linePosition;
                    fieldPosition++;
                }
            }
            linePosition++;
        }
        
        // Don't forget the last field (after the last comma)
        if (fieldCount < 3) {
            csvFields[fieldCount][fieldPosition] = '\0';
            fieldCount++;
        }
        
        // Validate that we got exactly 3 fields
        if (fieldCount != 3) {
            printf("Warning: Line %d in Exchange_Rates.csv has invalid format (got %d fields), skipping\n", lineNumber, fieldCount);
            continue;
        }
        
        // Parse each field with proper validation
        // Field 0: Date
        strncpy(currentRecord.date, csvFields[0], 9);
        currentRecord.date[9] = '\0';
        
        // Remove any trailing whitespace from date
        int dateLength = strlen(currentRecord.date);
        while (dateLength > 0 && (currentRecord.date[dateLength - 1] == ' ' || 
               currentRecord.date[dateLength - 1] == '\t' || currentRecord.date[dateLength - 1] == '\r' || 
               currentRecord.date[dateLength - 1] == '\n')) {
            currentRecord.date[dateLength - 1] = '\0';
            dateLength--;
        }
        
        // Field 1: Currency
        strncpy(currentRecord.currency, csvFields[1], 3);
        currentRecord.currency[3] = '\0';
        
        // Remove any trailing whitespace from currency
        int currencyLength = strlen(currentRecord.currency);
        while (currencyLength > 0 && (currentRecord.currency[currencyLength - 1] == ' ' || 
               currentRecord.currency[currencyLength - 1] == '\t' || currentRecord.currency[currencyLength - 1] == '\r' || 
               currentRecord.currency[currencyLength - 1] == '\n')) {
            currentRecord.currency[currencyLength - 1] = '\0';
            currencyLength--;
        }
        
        // Validate currency code
        if (ValidateCurrencyCode(currentRecord.currency) == 0) {
            printf("Warning: Line %d has invalid currency code '%s', skipping\n", lineNumber, currentRecord.currency);
            continue;
        }
        
        // Field 2: Exchange Rate
        if (sscanf(csvFields[2], "%lf", &currentRecord.exchange) != 1) {
            printf("Warning: Line %d has invalid exchange rate '%s', skipping\n", lineNumber, csvFields[2]);
            continue;
        }
        
        // Validate exchange rate is positive
        if (currentRecord.exchange <= 0.0) {
            printf("Warning: Line %d has invalid exchange rate '%lf' (must be positive), skipping\n", lineNumber, currentRecord.exchange);
            continue;
        }
        
        // Write the record to binary file
        if (fwrite(&currentRecord, sizeof(exchangeRateRecord), 1, binaryFilePointer) != 1) {
            printf("Error: Failed to write exchange rate record %d to binary file\n", recordCount + 1);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            recordCount++;
        }
    }
    
    if (errorOccurred == 0) {
        printf("Exchange Rates conversion completed: %d records processed\n", recordCount);
        returnValue = recordCount;
    }
    
    return returnValue;                                // Single return point
}//end function definition ConvertExchangeRatesCsvToBinary

/*
 * Function: ParseCurrencyFromCsv
 * Purpose: Parses currency string from CSV (removes $ and spaces)
 * Parameters: currencyString - currency string from CSV (e.g., "$6.62 ")
 *            parsedValue - pointer to double to store result
 * Returns: int - 1 if successful, 0 if parsing failed
 * Note: Handles currency format with $ prefix and trailing spaces
 *       Single return pattern, no break statements, uses double precision
 */
int ParseCurrencyFromCsv(const char* currencyString, double* parsedValue) {
    char cleanCurrency[20] = {0};                      // Buffer for cleaned currency string
    int cleanIndex = 0;                                // Index for clean string
    int isValid = 1;                                   // Validation flag (single return pattern)
    int continueProcessing = 1;                        // Control flag for loop
    
    // Initialize output to zero
    *parsedValue = 0.0;
    
    // Skip leading $ and spaces
    const char* position = currencyString;
    while (*position != '\0' && (*position == '$' || *position == ' ')) {
        position++;
    }
    
    // Copy digits and decimal point to clean string
    while (*position != '\0' && cleanIndex < 19 && continueProcessing == 1) {
        if ((*position >= '0' && *position <= '9') || *position == '.') {
            cleanCurrency[cleanIndex++] = *position;
        } else if (*position == ' ' || *position == '\t' || *position == '\r' || *position == '\n') {
            continueProcessing = 0;                    // Stop at trailing whitespace (no break)
        } else {
            // Invalid character encountered
            isValid = 0;
            continueProcessing = 0;
        }
        if (continueProcessing == 1) {
            position++;
        }
    }
    
    // Check for buffer overflow
    if (cleanIndex >= 19) {
        isValid = 0;                                   // String too long
    }
    
    cleanCurrency[cleanIndex] = '\0';
    
    // Parse the cleaned string
    if (sscanf(cleanCurrency, "%lf", parsedValue) != 1) {
        isValid = 0;                                   // Parsing failed
    }
    
    // Validate that the value is non-negative
    if (isValid == 1 && *parsedValue < 0.0) {
        isValid = 0;                                   // Invalid negative currency
    }
    
    return isValid;                                    // Single return point
}//end function definition ParseCurrencyFromCsv

/*
 * Function: ConvertProductsCsvToBinary
 * Purpose: Reads Products.csv and converts it to binary format
 * Parameters: csvFilePointer - pointer to opened Products.csv file
 *            binaryFilePointer - pointer to opened binary .dat file for writing
 * Returns: int - number of records converted, -1 if error
 * Note: Skips header line and validates each record, handles currency parsing
 */
int ConvertProductsCsvToBinary(FILE* csvFilePointer, FILE* binaryFilePointer) {
    char csvLineBuffer[1024] = {0};                    // Buffer for reading CSV lines
    productRecord currentRecord;                       // Current record being processed
    int recordCount = 0;                               // Number of successfully processed records
    int lineNumber = 0;                                // Current line number for error reporting
    int errorOccurred = 0;                             // Error flag (single return pattern)
    int returnValue = 0;                               // Return value (single return pattern)
    int headerRead = 0;                                // Flag for header reading success
    
    // Skip header line
    headerRead = (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL) ? 1 : 0;
    if (headerRead == 0) {
        printf("Error: Products.csv is empty or cannot be read\n");
        errorOccurred = 1;
        returnValue = -1;
    } else {
        lineNumber++;
    }
    
    // Process each data line
    while (fgets(csvLineBuffer, sizeof(csvLineBuffer), csvFilePointer) != NULL && errorOccurred == 0) {
        lineNumber++;
        
        // Initialize record to zero to prevent garbage data
        InitializeStructureToZero(&currentRecord, sizeof(productRecord));
        
        // Parse CSV line into fields
        char csvFields[10][100];                       // Array to store extracted fields (larger for product names)
        int fieldCount = 0;                            // Number of fields found
        
        // Initialize all fields to empty strings
        for (int i = 0; i < 10; i++) {
            csvFields[i][0] = '\0';
        }
        
        // Parse CSV line character by character to handle empty fields
        char* linePosition = csvLineBuffer;            // Current position in line
        int fieldPosition = 0;                         // Position within current field
        
        while (*linePosition != '\0' && *linePosition != '\n' && *linePosition != '\r' && fieldCount < 10) {
            if (*linePosition == ',') {
                // End of field - null terminate and move to next field
                csvFields[fieldCount][fieldPosition] = '\0';
                fieldCount++;
                fieldPosition = 0;
            } else {
                // Add character to current field if there's space
                if (fieldPosition < 99) {
                    csvFields[fieldCount][fieldPosition] = *linePosition;
                    fieldPosition++;
                }
            }
            linePosition++;
        }
        
        // Don't forget the last field (after the last comma)
        if (fieldCount < 10) {
            csvFields[fieldCount][fieldPosition] = '\0';
            fieldCount++;
        }
        
        // Validate that we got exactly 10 fields
        if (fieldCount != 10) {
            printf("Warning: Line %d in Products.csv has invalid format (got %d fields), skipping\n", lineNumber, fieldCount);
            continue;
        }
        
        // Parse each field with proper validation
        // Field 0: ProductKey
        if (sscanf(csvFields[0], "%hu", &currentRecord.productKey) != 1) {
            printf("Warning: Line %d has invalid product key '%s', skipping\n", lineNumber, csvFields[0]);
            continue;
        }
        
        // Field 1: Product Name
        strncpy(currentRecord.productName, csvFields[1], 29);
        currentRecord.productName[29] = '\0';
        
        // Field 2: Brand
        strncpy(currentRecord.brand, csvFields[2], 29);
        currentRecord.brand[29] = '\0';
        
        // Field 3: Color
        strncpy(currentRecord.color, csvFields[3], 14);
        currentRecord.color[14] = '\0';
        
        // Field 4: Unit Cost USD
        if (ParseCurrencyFromCsv(csvFields[4], &currentRecord.unitCostUSD) == 0) {
            printf("Warning: Line %d has invalid unit cost '%s', skipping\n", lineNumber, csvFields[4]);
            continue;
        }
        
        // Field 5: Unit Price USD
        if (ParseCurrencyFromCsv(csvFields[5], &currentRecord.unitPriceUSD) == 0) {
            printf("Warning: Line %d has invalid unit price '%s', skipping\n", lineNumber, csvFields[5]);
            continue;
        }
        
        // Field 6: SubcategoryKey
        strncpy(currentRecord.subcategoryKey, csvFields[6], 3);
        currentRecord.subcategoryKey[3] = '\0';
        
        // Field 7: Subcategory
        strncpy(currentRecord.subcategory, csvFields[7], 9);
        currentRecord.subcategory[9] = '\0';
        
        // Field 8: CategoryKey
        strncpy(currentRecord.categoryKey, csvFields[8], 1);
        currentRecord.categoryKey[1] = '\0';
        
        // Field 9: Category
        strncpy(currentRecord.category, csvFields[9], 19);
        currentRecord.category[19] = '\0';
        
        // Remove trailing whitespace from category
        int categoryLength = strlen(currentRecord.category);
        while (categoryLength > 0 && (currentRecord.category[categoryLength - 1] == ' ' || 
               currentRecord.category[categoryLength - 1] == '\t' || currentRecord.category[categoryLength - 1] == '\r' || 
               currentRecord.category[categoryLength - 1] == '\n')) {
            currentRecord.category[categoryLength - 1] = '\0';
            categoryLength--;
        }
        
        // Write the record to binary file
        if (fwrite(&currentRecord, sizeof(productRecord), 1, binaryFilePointer) != 1) {
            printf("Error: Failed to write product record %d to binary file\n", recordCount + 1);
            errorOccurred = 1;
            returnValue = -1;
        } else {
            recordCount++;
        }
    }
    
    if (errorOccurred == 0) {
        printf("Products conversion completed: %d records processed\n", recordCount);
        returnValue = recordCount;
    }
    
    return returnValue;                                // Single return point
}//end function definition ConvertProductsCsvToBinary

/*
 * Function: ConstructDatabaseTables
 * Purpose: Converts CSV files to binary format and sets up database tables
 * Parameters: salesFilePointer - pointer to sales CSV file
 *            customersFilePointer - pointer to customers CSV file
 *            exchangeRatesFilePointer - pointer to exchange rates CSV file
 *            productsFilePointer - pointer to products CSV file
 *            storesFilePointer - pointer to stores CSV file
 * Returns: void
 * Note: Main coordination function for database construction
 */
void ConstructDatabaseTables(
    FILE *salesFilePointer,
    FILE *customersFilePointer,
    FILE *exchangeRatesFilePointer,
    FILE *productsFilePointer,
    FILE *storesFilePointer
) {
    printf("Starting database construction from CSV files...\n");
    
    // Open binary files for writing
    FILE *salesBinaryFile = OpenFileWithErrorCheck("SalesTable.dat", "wb+");
    FILE *customersBinaryFile = OpenFileWithErrorCheck("CustomersTable.dat", "wb+");
    FILE *exchangeRatesBinaryFile = OpenFileWithErrorCheck("ExchangeRatesTable.dat", "wb+");
    FILE *productsBinaryFile = OpenFileWithErrorCheck("ProductsTable.dat", "wb+");
    FILE *storesBinaryFile = OpenFileWithErrorCheck("StoresTable.dat", "wb+");
    
    // Check if all binary files opened successfully
    if (salesBinaryFile == NULL || customersBinaryFile == NULL || 
        exchangeRatesBinaryFile == NULL || productsBinaryFile == NULL || 
        storesBinaryFile == NULL) {
        printf("Error: Could not create all binary files\n");
        // Close any files that did open
        if (salesBinaryFile != NULL) fclose(salesBinaryFile);
        if (customersBinaryFile != NULL) fclose(customersBinaryFile);
        if (exchangeRatesBinaryFile != NULL) fclose(exchangeRatesBinaryFile);
        if (productsBinaryFile != NULL) fclose(productsBinaryFile);
        if (storesBinaryFile != NULL) fclose(storesBinaryFile);
        return;
    }
    
    // Convert Sales CSV to binary
    int salesRecordCount = ConvertSalesCsvToBinary(salesFilePointer, salesBinaryFile);
    if (salesRecordCount < 0) {
        printf("Error: Sales conversion failed\n");
    }
    
    // Convert Customers CSV to binary
    int customersCount = ConvertCustomersCsvToBinary(customersFilePointer, customersBinaryFile);
    if (customersCount < 0) {
        printf("Error: Customers conversion failed\n");
    }
    
    // Convert Stores CSV to binary
    int storesCount = ConvertStoresCsvToBinary(storesFilePointer, storesBinaryFile);
    if (storesCount < 0) {
        printf("Error: Stores conversion failed\n");
    }
    
    // Convert Exchange Rates CSV to binary
    int exchangeRatesCount = ConvertExchangeRatesCsvToBinary(exchangeRatesFilePointer, exchangeRatesBinaryFile);
    if (exchangeRatesCount < 0) {
        printf("Error: Exchange Rates conversion failed\n");
    }
    
    // Convert Products CSV to binary
    int productsCount = ConvertProductsCsvToBinary(productsFilePointer, productsBinaryFile);
    if (productsCount < 0) {
        printf("Error: Products conversion failed\n");
    }
    
    // Close all binary files
    fclose(salesBinaryFile);
    fclose(customersBinaryFile);
    fclose(exchangeRatesBinaryFile);
    fclose(productsBinaryFile);
    fclose(storesBinaryFile);
    
    // Close CSV files
    fclose(storesFilePointer);
    fclose(productsFilePointer);
    fclose(exchangeRatesFilePointer);
    fclose(customersFilePointer);
    fclose(salesFilePointer);
    
    printf("Database construction completed.\n");
    return;
} // end function definition ConstructDatabaseTables

/*
 * Function: ShowMainMenu
 * Purpose: Displays the main menu options to the user
 * Parameters: None
 * Returns: void
 * Note: Menu format exactly as specified in assignment requirements
 */
void ShowMainMenu(void) {
    printf(
        "Company Global Electronics Retailer\n"
        "Options menu\n"
        "0. Exit program\n"
        "1. Construction of the Database with the dataset tables\n"
        "2. List of ¿What types of products does the company sell, and where are customers located?\n"
        "\t2.1 Utility bubbleSort\n"
        "\t2.2 Utility mergeSort\n"
        "3. List of ¿Are there any seasonal patterns or trends for order volume or revenue?\n"
        "\t3.1 Utility bubbleSort\n"
        "\t3.2 Utility mergeSort\n"
        "4. List of ¿How long is the average delivery time in days? Has that changed over time?\n"
        "\t4.1 Utility bubbleSort\n"
        "\t4.2 Utility mergeSort\n"
        "5. List of sales order by \"Costumer Name\"+\"Order Date\"+\"ProductKey\";\n"
        "\t5.1 Utility bubbleSort\n"
        "\t5.2 Utility mergeSort\n"
        "6. TEST: Sort Tables (temporary option for testing)\n"
        "\t6.1 Test Sales Bubble Sort\n"
        "\t6.2 Test Sales Merge Sort\n"
        "\t6.3 Test Stores Bubble Sort (small dataset)\n"
        "\t6.4 Test Stores Merge Sort (small dataset)\n"
        "7. TEST: Binary Search (search for ProductKey in sorted data)\n"
        "What is your option: "
    );
    return;
}//end function definition DisplayMainMenu

/*
 * Function: ExecuteMainProgramLoop
 * Purpose: Main program execution loop that handles user menu selection
 * Parameters: None
 * Returns: void
 * Note: Continues until user selects exit option (0)
 */
void ExecuteMainProgramLoop() {
    double selectedOption = -1.0;                      // User's menu choice selection (using double for precision)
    int mainOption = 0;                                // Main option (integer part)
    int subOption = 0;                                 // Sub option (decimal part)

    while (1) {
        ClearOutput();
        ShowMainMenu();

        if ((scanf("%lf", &selectedOption) != 1) || selectedOption < 0.0 || selectedOption > 7.0) {
            printf("Invalid option. Please try again.\n");
            while (getchar() != '\n');                 // Clean input buffer to prevent infinite loop
            system("pause");
        }
        else {
            // Parse main and sub options
            mainOption = (int)selectedOption;          // Integer part
            subOption = (int)((selectedOption - mainOption) * 10.0 + 0.5); // Decimal part
            
            if (selectedOption == 0.0) {
                return;                                // Exit the loop if the user chooses to exit
            }
        else if (mainOption == 1 && subOption == 0)  // Database construction from CSV files
        {
            // Open source CSV files for reading with error checking
            FILE *salesCsvFile = OpenFileWithErrorCheck("Sales.csv", "r");              // Sales transaction data
            FILE *customersCsvFile = OpenFileWithErrorCheck("Customers.csv", "r");          // Customer information data
            FILE *exchangeRatesCsvFile = OpenFileWithErrorCheck("Exchange_Rates.csv", "r");   // Currency exchange rates data
            FILE *productsCsvFile = OpenFileWithErrorCheck("Products.csv", "r");           // Product catalog data
            FILE *storesCsvFile = OpenFileWithErrorCheck("Stores.csv", "r");             // Store location data

            // Check if all CSV files opened successfully
            if (salesCsvFile != NULL && customersCsvFile != NULL && 
                exchangeRatesCsvFile != NULL && productsCsvFile != NULL && 
                storesCsvFile != NULL) {
                
                // All files opened successfully, proceed with conversion
                ConstructDatabaseTables(salesCsvFile, customersCsvFile, exchangeRatesCsvFile, productsCsvFile, storesCsvFile);
                
            } else {
                printf("Error: Could not open all required CSV files\n");
                // Close any files that did open
                if (salesCsvFile != NULL) fclose(salesCsvFile);
                if (customersCsvFile != NULL) fclose(customersCsvFile);
                if (exchangeRatesCsvFile != NULL) fclose(exchangeRatesCsvFile);
                if (productsCsvFile != NULL) fclose(productsCsvFile);
                if (storesCsvFile != NULL) fclose(storesCsvFile);
            }
            system("pause");
        }
        else if (mainOption == 2)  // Report: Product types and customer locations
        {
            if (subOption == 1) {
                // Option 2.1: Use Bubble Sort
                GenerateReport2ProductTypesAndLocations("Bubble");
            } else if (subOption == 2) {
                // Option 2.2: Use Merge Sort
                GenerateReport2ProductTypesAndLocations("Merge");
            } else if (subOption == 0) {
                // Option 2: Ask user to choose sorting method
                int sortChoice = 0;
                printf("\nSelect sorting algorithm:\n");
                printf("1. Bubble Sort\n");
                printf("2. Merge Sort\n");
                printf("Your choice: ");
                
                if (scanf("%d", &sortChoice) == 1) {
                    if (sortChoice == 1) {
                        GenerateReport2ProductTypesAndLocations("Bubble");
                    } else if (sortChoice == 2) {
                        GenerateReport2ProductTypesAndLocations("Merge");
                    } else {
                        printf("Invalid sorting choice.\n");
                    }
                } else {
                    printf("Invalid input.\n");
                    while (getchar() != '\n'); // Clean input buffer
                }
            } else {
                printf("Invalid sub-option for Report 2. Use 2.1 or 2.2\n");
            }
            system("pause");
        }
        else if (mainOption == 3)  // Report: Seasonal patterns analysis
        {
            printf("Report 3 not yet implemented. Coming soon!\n");
            // TODO: Implement seasonal patterns analysis report
            system("pause");
        }
        else if (mainOption == 4)  // Report: Average delivery time analysis
        {
            printf("Report 4 not yet implemented. Coming soon!\n");
            // TODO: Implement delivery time analysis report
            system("pause");
        }
        else if (mainOption == 5)  // Report: Customer sales listing
        {
            printf("Report 5 not yet implemented. Coming soon!\n");
            // TODO: Implement customer sales listing with currency conversion
            system("pause");
        }
        else if (mainOption == 6)  // TEST: Sort Tables
        {
            int testSubOption = 0;                     // Sub-menu selection
            printf("\nSelect sorting test:\n");
            printf("1. Sales Bubble Sort\n");
            printf("2. Sales Merge Sort\n");
            printf("3. Stores Bubble Sort (small dataset)\n");
            printf("4. Stores Merge Sort (small dataset)\n");
            printf("Your choice: ");
            
            if (scanf("%d", &testSubOption) == 1) {
                if (testSubOption == 1) {
                    TestSortSalesTable("Bubble");
                } else if (testSubOption == 2) {
                    TestSortSalesTable("Merge");
                } else if (testSubOption == 3) {
                    TestSortStoresTable("Bubble");
                } else if (testSubOption == 4) {
                    TestSortStoresTable("Merge");
                } else {
                    printf("Invalid sub-option selected.\n");
                }
            } else {
                printf("Invalid input.\n");
                while (getchar() != '\n'); // Clean input buffer
            }
            system("pause");
        }
        else if (mainOption == 7 && subOption == 0)  // TEST: Binary Search
        {
            TestBinarySearch();
            system("pause");
        }
        else {
            printf("Invalid option selected. Please try again.\n");
            system("pause");
        }
        } // End of main else block
    }
}//end function definition ExecuteMainProgramLoop

/*
 * Function: main
 * Purpose: Main entry point of the program
 * Parameters: void
 * Returns: int - exit status (0 for successful execution)
 * Note: Sets up console encoding and starts the main program loop
 */
int main(void) {
    SetConsoleOutputCP(CP_UTF8);          // Enable UTF-8 support for console output
    ExecuteMainProgramLoop();
    printf("Thanks for using our app, see you next time!\n");
    system("pause");
    return 0;
}//end function definition main
