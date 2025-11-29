/*
 * Personal Finance Manager - C++ Single File
 * -------------------------------------------
 * Features:
 * - Add, delete, and list transactions
 * - Save and load transactions from a CSV file
 * - Search and sort transactions
 * - Monthly income/expense summary
 * - Budget categories with alerts
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <limits>
#include <cctype>

 // --------------------------------------------------------------------
 // ---------------------------- UTILITIES ------------------------------
 // --------------------------------------------------------------------

 // Removes whitespace from the beginning and end of a string.
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Checks if a string represents a valid number (integer or floating point).
bool isNumber(const std::string& s) {
    std::istringstream iss(s);
    double d;
    char c;

    // First check: can we read a number?
    if (!(iss >> d)) return false;

    // Second check: ensure no extra characters remain.
    if (iss >> c) return false;

    return true;
}

// Validates the date format "YYYY-MM-DD".
bool validateDate(const std::string& date) {
    // Basic format validation.
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;

    // Validate that all characters except '-' are digits.
    for (size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(date[i])) return false;
    }

    // Basic range validation (not a full calendar validation).
    try {
        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));

        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        if (year < 1900 || year > 2100) return false;
    }
    catch (...) {
        return false;
    }

    return true;
}

// Reads an integer with full validation and range control.
int readInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);

        try {
            value = std::stoi(line);
            if (value < min || value > max) {
                std::cout << "Please enter a number between " << min << " and " << max << ".\n";
                continue;
            }
            return value;
        }
        catch (...) {
            std::cout << "Invalid input. Try again.\n";
        }
    }
}

// Reads a double value with validation.
double readDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);

        try {
            value = std::stod(line);
            return value;
        }
        catch (...) {
            std::cout << "Invalid input. Try again.\n";
        }
    }
}

// Pauses the screen until the user presses ENTER.
void pause() {
    std::cout << "Press ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// --------------------------------------------------------------------
// ---------------------------- CLASSES --------------------------------
// --------------------------------------------------------------------

// Represents a single financial transaction.
class Transaction {
private:
    std::string date;        // Date of transaction (YYYY-MM-DD)
    std::string category;    // Category (Food, Rent, Salary, etc.)
    double amount;           // Positive = income, Negative = expense
    std::string description; // Extra details

public:
    Transaction() : date(""), category(""), amount(0), description("") {}

    // Full constructor
    Transaction(const std::string& d, const std::string& c, double a, const std::string& desc)
        : date(d), category(c), amount(a), description(desc) {}

    // Getters
    std::string getDate() const { return date; }
    std::string getCategory() const { return category; }
    double getAmount() const { return amount; }
    std::string getDescription() const { return description; }

    // Returns a formatted string to print the transaction.
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setw(10) << date << " | "
            << std::setw(15) << category << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << amount << " | "
            << description;
        return oss.str();
    }
};

// Stores a budget category with a spending limit.
class Budget {
private:
    std::string category;
    double limit;

public:
    Budget() : category(""), limit(0) {}

    Budget(const std::string& c, double l)
        : category(c), limit(l) {}

    std::string getCategory() const { return category; }
    double getLimit() const { return limit; }

    void setLimit(double l) { limit = l; }
};

// Main class managing all data: transactions + budgets.
class FinanceManager {
private:
    std::vector<Transaction> transactions;
    std::vector<Budget> budgets;

public:
    FinanceManager() {}

    // Returns the number of transactions (used when deleting).
    size_t getSize() const {
        return transactions.size();
    }

    // Adds a new transaction.
    void addTransaction(const Transaction& t) {
        transactions.push_back(t);
        std::cout << "Transaction added successfully.\n";
    }

    // Removes a transaction by index.
    bool deleteTransaction(int index) {
        if (index < 0 || index >= static_cast<int>(transactions.size()))
            return false;

        transactions.erase(transactions.begin() + index);
        std::cout << "Transaction deleted successfully.\n";
        return true;
    }

    // Displays all recorded transactions.
    void listTransactions() const {
        if (transactions.empty()) {
            std::cout << "No transactions recorded.\n";
            return;
        }

        std::cout << "Idx | Date        | Category       |    Amount | Description\n";
        std::cout << "-------------------------------------------------------------------\n";

        for (size_t i = 0; i < transactions.size(); ++i) {
            std::cout << std::setw(3) << i << " | " << transactions[i].toString() << "\n";
        }
    }

    // Writes all transactions into a CSV file.
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);

        if (!file) {
            std::cout << "Error opening file to save.\n";
            return;
        }

        for (const auto& t : transactions) {
            std::string desc = t.getDescription();
            std::replace(desc.begin(), desc.end(), ',', ';'); // Prevent CSV break

            file << t.getDate() << ","
                << t.getCategory() << ","
                << t.getAmount() << ","
                << desc << "\n";
        }

        file.close();
        std::cout << "Data saved to " << filename << "\n";
    }

    // Loads transactions from a CSV file.
    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);

        if (!file) {
            std::cout << "Error opening file to load.\n";
            return;
        }

        transactions.clear();
        std::string line;
        int lineCount = 0;

        while (getline(file, line)) {
            lineCount++;
            std::stringstream ss(line);

            std::string date, category, amountStr, description;

            getline(ss, date, ',');
            getline(ss, category, ',');
            getline(ss, amountStr, ',');
            getline(ss, description);

            date = trim(date);
            category = trim(category);
            amountStr = trim(amountStr);
            description = trim(description);

            if (!validateDate(date)) {
                std::cout << "Invalid date format on line " << lineCount << ". Skipping.\n";
                continue;
            }

            if (!isNumber(amountStr)) {
                std::cout << "Invalid amount on line " << lineCount << ". Skipping.\n";
                continue;
            }

            double amount = stod(amountStr);

            transactions.push_back(Transaction(date, category, amount, description));
        }

        file.close();
        std::cout << "File loaded with " << transactions.size() << " transactions.\n";
    }

    // Prints a summary of income, expenses and net balance for a specific month.
    void monthlySummary(const std::string& yearMonth) const {
        if (yearMonth.length() != 7 || yearMonth[4] != '-') {
            std::cout << "Invalid format, must be YYYY-MM.\n";
            return;
        }

        double income = 0, expense = 0;

        // Loop through all transactions of the specified month.
        for (const auto& t : transactions) {
            if (t.getDate().substr(0, 7) == yearMonth) {
                if (t.getAmount() >= 0) income += t.getAmount();
                else expense += t.getAmount();
            }
        }

        std::cout << "\nSummary for " << yearMonth << ":\n";
        std::cout << "Income:   $" << std::fixed << std::setprecision(2) << income << "\n";
        std::cout << "Expenses: $" << std::fixed << std::setprecision(2) << expense << "\n";
        std::cout << "Net:      $" << std::fixed << std::setprecision(2) << (income + expense) << "\n";
    }

    // Searches transactions either by category or exact date.
    void searchTransactions() const {
        std::cout << "Search by:\n1. Category (substring)\n2. Exact date (YYYY-MM-DD)\nOption: ";
        std::string optStr;
        std::getline(std::cin, optStr);

        int opt = -1;
        try { opt = std::stoi(optStr); }
        catch (...) { std::cout << "Invalid option.\n"; return; }

        if (opt == 1) {
            std::cout << "Enter part of the category to search: ";
            std::string query;
            std::getline(std::cin, query);

            bool found = false;

            for (size_t i = 0; i < transactions.size(); ++i) {
                if (transactions[i].getCategory().find(query) != std::string::npos) {
                    if (!found) {
                        std::cout << "Results found:\n";
                        std::cout << "Idx | Date        | Category       |    Amount | Description\n";
                        std::cout << "-------------------------------------------------------------------\n";
                    }

                    std::cout << std::setw(3) << i << " | " << transactions[i].toString() << "\n";
                    found = true;
                }
            }

            if (!found)
                std::cout << "No transactions found for that category.\n";
        }
        else if (opt == 2) {
            std::cout << "Enter exact date (YYYY-MM-DD): ";
            std::string date;
            std::getline(std::cin, date);

            if (!validateDate(date)) {
                std::cout << "Invalid date.\n";
                return;
            }

            bool found = false;

            for (size_t i = 0; i < transactions.size(); ++i) {
                if (transactions[i].getDate() == date) {
                    if (!found) {
                        std::cout << "Results found:\n";
                        std::cout << "Idx | Date        | Category       |    Amount | Description\n";
                        std::cout << "-------------------------------------------------------------------\n";
                    }

                    std::cout << std::setw(3) << i << " | " << transactions[i].toString() << "\n";
                    found = true;
                }
            }

            if (!found)
                std::cout << "No transactions found on that date.\n";
        }
        else {
            std::cout << "Invalid option.\n";
        }
    }

    // Sorts transactions by date or by amount.
    void sortTransactions() {
        std::cout << "Sort by:\n1. Date ascending\n2. Amount ascending\nOption: ";
        std::string optStr;
        std::getline(std::cin, optStr);

        int opt = -1;
        try { opt = std::stoi(optStr); }
        catch (...) { std::cout << "Invalid option.\n"; return; }

        if (opt == 1) {
            std::sort(transactions.begin(), transactions.end(),
                [](const Transaction& a, const Transaction& b) {
                    return a.getDate() < b.getDate();
                });
            std::cout << "Transactions sorted by date ascending.\n";
        }
        else if (opt == 2) {
            std::sort(transactions.begin(), transactions.end(),
                [](const Transaction& a, const Transaction& b) {
                    return a.getAmount() < b.getAmount();
                });
            std::cout << "Transactions sorted by amount ascending.\n";
        }
        else {
            std::cout << "Invalid option.\n";
        }
    }

    // Allows user to add a new budget or update an existing one.
    void addOrUpdateBudget() {
        std::cout << "Enter category for budget: ";
        std::string cat;
        std::getline(std::cin, cat);
        cat = trim(cat);

        if (cat.empty()) {
            std::cout << "Category cannot be empty.\n";
            return;
        }

        std::cout << "Enter budget limit (positive number): ";
        double limit = readDouble("");

        if (limit < 0) {
            std::cout << "Limit cannot be negative.\n";
            return;
        }

        // Check if the budget already exists.
        for (auto& b : budgets) {
            if (b.getCategory() == cat) {
                b.setLimit(limit);
                std::cout << "Budget updated for category '" << cat << "'.\n";
                return;
            }
        }

        // Otherwise, add new category.
        budgets.push_back(Budget(cat, limit));
        std::cout << "Budget added for category '" << cat << "'.\n";
    }

    // Lists all defined budgets.
    void listBudgets() const {
        if (budgets.empty()) {
            std::cout << "No budgets defined.\n";
            return;
        }

        std::cout << "Category          | Limit\n";
        std::cout << "----------------------------\n";

        for (const auto& b : budgets) {
            std::cout << std::setw(18) << b.getCategory()
                << " | $"
                << std::fixed << std::setprecision(2)
                << b.getLimit() << "\n";
        }
    }

    // Checks if spending in each category exceeds the defined budget.
    void checkBudgets() const {
        if (budgets.empty()) {
            std::cout << "No budgets defined.\n";
            return;
        }

        // Map category → total spent.
        std::map<std::string, double> spentPerCategory;

        for (const auto& t : transactions) {
            if (t.getAmount() < 0) {
                spentPerCategory[t.getCategory()] += (-t.getAmount());
            }
        }

        bool anyExceeded = false;
        std::cout << "\nBudget check:\n";

        for (const auto& b : budgets) {
            double spent = spentPerCategory[b.getCategory()];

            if (spent > b.getLimit()) {
                std::cout << "ALERT! Category '" << b.getCategory()
                    << "' has exceeded the budget! Spent: $"
                    << spent << ", Limit: $" << b.getLimit() << "\n";
                anyExceeded = true;
            }
            else {
                std::cout << "Category '" << b.getCategory()
                    << "' is within budget. Spent: $"
                    << spent << ", Limit: $" << b.getLimit() << "\n";
            }
        }

        if (!anyExceeded) {
            std::cout << "All budgets are within limits.\n";
        }
    }

    bool isEmpty() const {
        return transactions.empty();
    }
};

// --------------------------------------------------------------------
// ---------------------------- MENU + MAIN ----------------------------
// --------------------------------------------------------------------

// Prints the main program menu.
void printMenu() {
    std::cout << "\n=== Personal Finance Manager ===\n";
    std::cout << "1. Add transaction\n";
    std::cout << "2. Delete transaction\n";
    std::cout << "3. List transactions\n";
    std::cout << "4. Save transactions to file\n";
    std::cout << "5. Load transactions from file\n";
    std::cout << "6. Monthly summary\n";
    std::cout << "7. Search transactions\n";
    std::cout << "8. Sort transactions\n";
    std::cout << "9. Add or update budget\n";
    std::cout << "10. List budgets\n";
    std::cout << "11. Check budgets\n";
    std::cout << "0. Exit\n";
    std::cout << "Select option: ";
}

// Collects all user inputs and creates a Transaction object.
Transaction inputTransaction() {
    std::string date, category, description;
    double amount;

    // Ask for date until format is valid.
    while (true) {
        std::cout << "Date (YYYY-MM-DD): ";
        std::getline(std::cin, date);

        if (validateDate(date))
            break;

        std::cout << "Invalid date, try again.\n";
    }

    // Ask for category.
    std::cout << "Category: ";
    std::getline(std::cin, category);
    category = trim(category);
    if (category.empty()) category = "Miscellaneous";

    // Ask for amount.
    while (true) {
        std::cout << "Amount (positive income, negative expense): ";
        std::string amtStr;
        std::getline(std::cin, amtStr);

        if (isNumber(amtStr)) {
            amount = std::stod(amtStr);
            break;
        }

        std::cout << "Invalid amount, try again.\n";
    }

    // Ask for optional description.
    std::cout << "Description: ";
    std::getline(std::cin, description);

    return Transaction(date, category, amount, description);
}

// Main program loop.
int main() {
    FinanceManager fm;
    bool running = true;

    while (running) {
        printMenu();

        std::string choiceStr;
        std::getline(std::cin, choiceStr);

        int choice = -1;
        try { choice = std::stoi(choiceStr); }
        catch (...) { choice = -1; }

        switch (choice) {
        case 1: {
            Transaction t = inputTransaction();
            fm.addTransaction(t);
            pause();
            break;
        }

        case 2: {
            if (fm.isEmpty()) {
                std::cout << "No transactions to delete.\n";
                pause();
                break;
            }

            fm.listTransactions();

            // Correctly calculate max index.
            int max_index = static_cast<int>(fm.getSize()) - 1;

            std::string prompt = "Enter transaction index to delete (0 to " +
                std::to_string(max_index) + "): ";

            int idx = readInt(prompt, 0, max_index);

            if (!fm.deleteTransaction(idx)) {
                std::cout << "Invalid index.\n";
            }

            pause();
            break;
        }

        case 3:
            fm.listTransactions();
            pause();
            break;

        case 4: {
            std::cout << "Enter filename to save (e.g. data.csv): ";
            std::string filename;
            std::getline(std::cin, filename);

            if (filename.empty()) filename = "data.csv";

            fm.saveToFile(filename);
            pause();
            break;
        }

        case 5: {
            std::cout << "Enter filename to load (e.g. data.csv): ";
            std::string filename;
            std::getline(std::cin, filename);

            if (filename.empty()) filename = "data.csv";

            fm.loadFromFile(filename);
            pause();
            break;
        }

        case 6: {
            std::cout << "Enter year and month for summary (format YYYY-MM): ";
            std::string ym;
            std::getline(std::cin, ym);

            fm.monthlySummary(ym);
            pause();
            break;
        }

        case 7:
            fm.searchTransactions();
            pause();
            break;

        case 8:
            fm.sortTransactions();
            pause();
            break;

        case 9:
            fm.addOrUpdateBudget();
            pause();
            break;

        case 10:
            fm.listBudgets();
            pause();
            break;

        case 11:
            fm.checkBudgets();
            pause();
            break;

        case 0:
            running = false;
            std::cout << "Exiting program...\n";
            break;

        default:
            std::cout << "Invalid option, please try again.\n";
            pause();
            break;
        }
    }

    return 0;
}
