/*
 * Publication Date: November 16, 2025
 * Time: 20:32
 * Version: 1.0.0
 * Author: Ing(c) Andres David Rincon Salazar
 * Language Used: C (ISO/IEC)
 * Language Version: C11
 * Presented to: Doctor Ricardo Moreno Laverde
 * Universidad Tecnológica de Pereira
 * Programa de Ingeniería de Sistemas y Computación
 *
 * File Description:
 * This header file contains all data structure definitions for the Global Electronics 
 * Retailer database system. All structures are designed with fixed-size fields to ensure
 * consistent binary file storage and efficient file-based operations.
 *
 * The structures correspond to the five main database tables:
 * - Sales: Transaction records with foreign key relationships
 * - Customers: Customer demographic and geographic information
 * - Products: Product catalog with pricing and categorization
 * - Stores: Store location and operational data
 * - Exchange Rates: Currency conversion rates by date
 *
 * All field sizes match exactly the data dictionary specifications from the
 * IS284 final project requirements.
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

// ====================== STANDARD LIBRARY INCLUDES ======================
#include <stddef.h>        // For size_t type definition
#include <limits.h>        // For ULONG_MAX and other limits

// ====================== DATE AND TIME STRUCTURES ======================

/*
 * Structure: dateStructure
 * Purpose: Represents a date with day, month, and year components
 * Fields: dayOfMonth - day component (1-31)
 *         monthOfYear - month component (1-12)
 *         yearValue - year component (full 4-digit year)
 * Size: 4 bytes (1 + 1 + 2)
 * Note: Compact binary representation using minimal storage
 */
typedef struct {
    unsigned char dayOfMonth;              // Day of the month (1-31)
    unsigned char monthOfYear;             // Month of the year (1-12)
    unsigned short yearValue;              // Four-digit year value
} dateStructure;

// ====================== DATABASE TABLE STRUCTURES ======================

/*
 * Structure: salesRecord
 * Purpose: Represents a sales transaction record with all order details
 * Fields: orderNumber - Unique ID for each order
 *         lineItem - Identifies individual products purchased as part of an order
 *         orderDate - Date the order was placed
 *         deliveryDate - Date the order was delivered
 *         customerKey - Unique key identifying which customer placed the order
 *         storeKey - Unique key identifying which store processed the order
 *         productKey - Unique key identifying which product was purchased
 *         quantity - Number of items purchased
 *         currencyCode - Currency used to process the order
 * Size: ~28 bytes
 * Note: Core transaction table with relationships to all other tables
 */
typedef struct SalesTable {
    long orderNumber;                      // Unique ID for each order
    unsigned char lineItem;                // Identifies individual products purchased
    dateStructure orderDate;               // Date the order was placed
    dateStructure deliveryDate;            // Date the order was delivered
    unsigned int customerKey;              // Customer foreign key
    unsigned short storeKey;               // Store foreign key
    unsigned short productKey;             // Product foreign key
    unsigned short quantity;               // Number of items purchased
    char currencyCode[4];                  // Currency code (3 chars + null terminator)
} salesRecord;

/*
 * Structure: customerRecord
 * Purpose: Represents customer demographic and geographic information
 * Fields: customerKey - Primary key to identify customers
 *         gender - Customer gender
 *         name - Customer full name
 *         city - Customer city
 *         stateCode - Customer state (abbreviated)
 *         state - Customer state (full)
 *         zipCode - Customer zip code
 *         country - Customer country
 *         continent - Customer continent
 *         birthday - Customer date of birth
 * Size: ~202 bytes
 * Note: Contains all customer demographic and location data for analysis
 */
typedef struct CustomersTable {
    unsigned int customerKey;              // Primary key to identify customers
    char gender[8];                        // Customer gender
    char name[40];                         // Customer full name
    char city[40];                         // Customer city
    char stateCode[20];                    // Customer state (abbreviated)
    char state[30];                        // Customer state (full)
    unsigned int zipCode;                  // Customer zip code
    char country[20];                      // Customer country
    char continent[20];                    // Customer continent
    dateStructure birthday;                // Customer date of birth
} customerRecord;

/*
 * Structure: productRecord
 * Purpose: Represents product catalog information with pricing and categorization
 * Fields: productKey - Primary key to identify products
 *         productName - Product name
 *         brand - Product brand
 *         color - Product color
 *         unitCostUSD - Cost to produce the product in USD
 *         unitPriceUSD - Product list price in USD
 *         subcategoryKey - Key to identify product subcategories
 *         subcategory - Product subcategory name
 *         categoryKey - Key to identify product categories
 *         category - Product category name
 * Size: ~114 bytes
 * Note: Contains complete product information for sales analysis
 */
typedef struct ProductsTable {
    unsigned short productKey;             // Primary key to identify products
    char productName[30];                  // Product name
    char brand[30];                        // Product brand
    char color[15];                        // Product color
    double unitCostUSD;                    // Cost to produce the product in USD
    double unitPriceUSD;                   // Product list price in USD
    char subcategoryKey[4];                // Product subcategory key
    char subcategory[10];                  // Product subcategory name
    char categoryKey[2];                   // Product category key
    char category[20];                     // Product category name
} productRecord;

/*
 * Structure: storeRecord
 * Purpose: Represents store location and operational information
 * Fields: storeKey - Primary key to identify stores
 *         country - Store country
 *         state - Store state
 *         squareMeters - Store footprint in square meters
 *         openDate - Store open date
 * Size: ~78 bytes
 * Note: Contains store geographic and operational data
 */
typedef struct StoresTable {
    unsigned short storeKey;               // Primary key to identify stores
    char country[35];                      // Store country
    char state[35];                        // Store state
    unsigned short squareMeters;           // Store footprint in square meters
    dateStructure openDate;                // Store open date
} storeRecord;

/*
 * Structure: exchangeRateRecord
 * Purpose: Represents currency exchange rates for conversion calculations
 * Fields: date - Date in DD/MM/YYYY format
 *         currency - Currency code
 *         exchange - Exchange rate compared to USD
 * Size: ~22 bytes
 * Note: Essential for converting sales in different currencies to USD
 */
typedef struct ExchangeRatesTable {
    char date[10];                         // Date (format: DD/MM/YYYY)
    char currency[4];                      // Currency code (3 chars + null terminator)
    double exchange;                       // Exchange rate compared to USD
} exchangeRateRecord;

// ====================== REPORT AND ANALYSIS STRUCTURES ======================

/*
 * Structure: monthlySalesData
 * Purpose: Stores aggregated sales data for each month for seasonal analysis
 * Fields: year - year of the data
 *         month - month (1-12)
 *         orderCount - total number of orders in that month
 *         totalRevenue - total revenue in USD for that month
 * Size: ~16 bytes
 * Note: Used for Report 3 - Seasonal patterns analysis
 */
typedef struct {
    unsigned short year;                   // Year (e.g., 2016)
    unsigned char month;                   // Month (1-12)
    unsigned long orderCount;              // Total orders in this month
    double totalRevenue;                   // Total revenue in USD
} monthlySalesData;

/*
 * Structure: categorySeasonalData
 * Purpose: Stores seasonal sales data by product category
 * Fields: category - product category name
 *         q1Revenue, q2Revenue, q3Revenue, q4Revenue - revenue by quarter
 *         q1Orders, q2Orders, q3Orders, q4Orders - order counts by quarter
 * Size: ~68 bytes
 * Note: Used for category-level seasonal analysis in Report 3
 */
typedef struct {
    char category[20];                     // Product category
    double q1Revenue;                      // Q1 (Jan-Mar) revenue
    double q2Revenue;                      // Q2 (Apr-Jun) revenue
    double q3Revenue;                      // Q3 (Jul-Sep) revenue
    double q4Revenue;                      // Q4 (Oct-Dec) revenue
    unsigned long q1Orders;                // Q1 order count
    unsigned long q2Orders;                // Q2 order count
    unsigned long q3Orders;                // Q3 order count
    unsigned long q4Orders;                // Q4 order count
} categorySeasonalData;

/*
 * Structure: regionSeasonalData
 * Purpose: Stores seasonal sales data by continent/region
 * Fields: continent - region name
 *         q1Revenue, q2Revenue, q3Revenue, q4Revenue - revenue by quarter
 *         q1Orders, q2Orders, q3Orders, q4Orders - order counts by quarter
 * Size: ~68 bytes
 * Note: Used for regional seasonal analysis in Report 3
 */
typedef struct {
    char continent[20];                    // Continent/region
    double q1Revenue;                      // Q1 (Jan-Mar) revenue
    double q2Revenue;                      // Q2 (Apr-Jun) revenue
    double q3Revenue;                      // Q3 (Jul-Sep) revenue
    double q4Revenue;                      // Q4 (Oct-Dec) revenue
    unsigned long q1Orders;                // Q1 order count
    unsigned long q2Orders;                // Q2 order count
    unsigned long q3Orders;                // Q3 order count
    unsigned long q4Orders;                // Q4 order count
} regionSeasonalData;

/*
 * Structure: productCustomerRecord
 * Purpose: Combined record for Report 2 - Products and customer locations
 * Fields: product - product information
 *         customer - customer information
 * Size: ~316 bytes (114 + 202)
 * Note: Used for joined data in Report 2 analysis
 */
typedef struct {
    productRecord product;                 // Product information
    customerRecord customer;               // Customer information
} productCustomerRecord;

/*
 * Structure: salesCustomerRecord
 * Purpose: Combined record for Report 5 - Sales listings by customer
 * Fields: sale - sales transaction information
 *         customer - customer information
 * Size: ~230 bytes (28 + 202)
 * Note: Used for joined data in Report 5 analysis
 */
typedef struct {
    salesRecord sale;                      // Sales information
    customerRecord customer;               // Customer information
} salesCustomerRecord;

// ====================== FILE-BASED LINKED LIST STRUCTURES ======================

/*
 * Structure: DoublyLinkedNodeHeader
 * Purpose: Header structure for file-based doubly linked list nodes
 * Fields: prevOffset - file offset to previous node (-1 if head)
 *         nextOffset - file offset to next node (-1 if tail)
 *         dataSize - size of the actual data payload
 * Size: 16 bytes (8 + 8)
 * Note: Enables efficient sorting without loading entire dataset to RAM
 *       Actual record data immediately follows this header in file
 *       All list operations performed directly on file
 */
typedef struct DoublyLinkedNodeHeader {
    long prevOffset;                       // File offset to previous node (-1 = NULL/head)
    long nextOffset;                       // File offset to next node (-1 = NULL/tail)
    size_t dataSize;                       // Size of the data payload in bytes
} DoublyLinkedNodeHeader;

/*
 * Structure: LinkedListFileMetadata
 * Purpose: Metadata stored at the beginning of linked list file
 * Fields: headOffset - file offset to first node
 *         tailOffset - file offset to last node
 *         nodeCount - total number of nodes in list
 *         recordSize - size of each record's data (excluding node header)
 * Size: 32 bytes
 * Note: Always stored at position 0 of the linked list file
 *       Provides O(1) access to list boundaries and size
 */
typedef struct LinkedListFileMetadata {
    long headOffset;                       // Offset to first node (-1 if empty)
    long tailOffset;                       // Offset to last node (-1 if empty)
    long nodeCount;                        // Total number of nodes
    size_t recordSize;                     // Size of data payload per node
} LinkedListFileMetadata;

// ====================== UTILITY FUNCTIONS ======================

/*
 * Function: RoundToThirdDecimal
 * Purpose: Rounds a double value to 3 decimal places using 5/4 rounding rule
 * Parameters: value - the double value to round
 * Returns: double - rounded value
 * Note: 5/4 rule: if digit at position 4 is >=5, round up the third decimal
 *       Example: 1.23456 -> 1.235, 1.23449 -> 1.234
 *       Declared as static inline for efficiency (no function call overhead)
 */
static inline double RoundToThirdDecimal(double value) {
    double multiplier = 1000.0;            // Multiplier to move decimal point
    double roundedValue = 0.0;             // Final rounded value
    double tempValue = 0.0;                // Temporary value for calculation
    
    tempValue = value * multiplier;        // Move decimal point 3 places right
    
    // Apply 5/4 rounding: floor(x + 0.5)
    if (tempValue >= 0.0) {
        roundedValue = (double)((long long)(tempValue + 0.5));
    } else {
        roundedValue = (double)((long long)(tempValue - 0.5));
    }
    
    roundedValue = roundedValue / multiplier; // Move decimal point back
    
    return roundedValue;
}//end function definition RoundToThirdDecimal

#endif // STRUCTURES_H
