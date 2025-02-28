#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <ctime>

using namespace std;

class Book {
public:
    int id;
    char title[50];
    char author[50];
    int copies;

    static int generateBookID() {
        ifstream fin("book_id_counter.dat");
        int lastId = 0;
        if (fin) {
            fin >> lastId;
            fin.close();
        }
        lastId++;
        ofstream fout("book_id_counter.dat");
        fout << lastId;
        fout.close();
        return lastId;
    }

    void getData() {
        id = generateBookID();  
        cin.ignore();
        cout << "Enter Title: ";
        cin.getline(title, 50);
        cout << "Enter Author: ";
        cin.getline(author, 50);
        cout << "Enter Number of Copies: ";
        cin >> copies;
        cin.ignore();
        cout << "Book added with ID: " << id << endl;
    }

    void display() const {
        cout << left << setw(10) << id << setw(30) << title << setw(30) << author << setw(10) << copies << endl;
    }
};

class IssuedBook {
public:
    int bookId;
    char userName[50];
    char studentId[20];

    void logTransaction(const string& action) {
        ofstream logFile("library_log.txt", ios::app);
        time_t now = time(0);
        tm *ltm = localtime(&now);
        logFile << "Time: " << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << " "
                << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << " | "
                << "Action: " << action << ", User: " << userName << ", Student ID: " << studentId << ", Book ID: " << bookId << endl;
        logFile.close();
    }

    void issueBook() {
        cout << "Enter your name: ";
        cin.ignore();
        cin.getline(userName, 50);
        cout << "Enter your Student ID: ";
        cin.getline(studentId, 20);
        cout << "Enter Book ID to issue: ";
        cin >> bookId;

        ifstream fin("books.dat", ios::binary);
        ofstream fout("issued_books.dat", ios::binary | ios::app);
        Book book;
        bool found = false;
        vector<Book> books;

        while (fin.read(reinterpret_cast<char*>(&book), sizeof(book))) {
            if (book.id == bookId && book.copies > 0) {
                found = true;
                book.copies--;
                fout.write(reinterpret_cast<char*>(this), sizeof(IssuedBook));
            }
            books.push_back(book);
        }
        fin.close();
        fout.close();

        if (found) {
            ofstream bookOut("books.dat", ios::binary);
            for (const auto& b : books) {
                bookOut.write(reinterpret_cast<const char*>(&b), sizeof(b));
            }
            bookOut.close();
            cout << "Book issued successfully!\n";
            logTransaction("Issued");
        } else {
            cout << "Book not available or invalid ID!\n";
        }
    }

    void returnBook() {
        cout << "Enter your name: ";
        cin.ignore();
        cin.getline(userName, 50);
        cout << "Enter your Student ID: ";
        cin.getline(studentId, 20);
        cout << "Enter Book ID to return: ";
        cin >> bookId;

        ifstream fin("issued_books.dat", ios::binary);
        ofstream fout("temp.dat", ios::binary);
        IssuedBook issued;
        bool found = false;

        while (fin.read(reinterpret_cast<char*>(&issued), sizeof(issued))) {
            if (issued.bookId == bookId && strcmp(issued.userName, userName) == 0 && strcmp(issued.studentId, studentId) == 0) {
                found = true;
            } else {
                fout.write(reinterpret_cast<char*>(&issued), sizeof(issued));
            }
        }
        fin.close();
        fout.close();

        if (found) {
            remove("issued_books.dat");
            rename("temp.dat", "issued_books.dat");

            ifstream bookIn("books.dat", ios::binary);
            vector<Book> books;
            Book book;
            while (bookIn.read(reinterpret_cast<char*>(&book), sizeof(book))) {
                if (book.id == bookId) {
                    book.copies++;
                }
                books.push_back(book);
            }
            bookIn.close();

            ofstream bookOut("books.dat", ios::binary);
            for (const auto& b : books) {
                bookOut.write(reinterpret_cast<const char*>(&b), sizeof(b));
            }
            bookOut.close();

            cout << "Book returned successfully by " << userName << " (Student ID: " << studentId << ")!\n";
            logTransaction("Returned");
        } else {
            cout << "No record found for " << userName << " (Student ID: " << studentId << ") issuing Book ID " << bookId << "!\n";
        }
    }
};



class Library {
public:
    void addBook() {
        Book book;
        book.getData();

        ofstream fout("books.dat", ios::binary | ios::app);
        fout.write((char*)&book, sizeof(book));
        fout.close();
    }

    void displayBooks() {
        ifstream fin("books.dat", ios::binary);
        Book book;
        cout << left << setw(10) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(10) << "Copies" << endl;
        cout << "-------------------------------------------------------------------------------\n";

        while (fin.read((char*)&book, sizeof(book))) {
            book.display();
        }
        fin.close();
    }

    void editBook(int bookId) {
        fstream file("books.dat", ios::binary | ios::in | ios::out);
        Book book;
        bool found = false;

        while (file.read((char*)&book, sizeof(book))) {
            if (book.id == bookId) {
                cout << "Editing book details for ID " << bookId << "\n";
                book.getData();
                file.seekp(-sizeof(book), ios::cur);
                file.write((char*)&book, sizeof(book));
                found = true;
                break;
            }
        }
        file.close();
        if (found) cout << "Book details updated successfully!\n";
        else cout << "Book not found!\n";
    }

    void deleteBook(int bookId) {
        ifstream fin("books.dat", ios::binary);
        ofstream fout("temp.dat", ios::binary);
        Book book;
        bool found = false;

        while (fin.read((char*)&book, sizeof(book))) {
            if (book.id == bookId) {
                found = true;
            } else {
                fout.write((char*)&book, sizeof(book));
            }
        }
        fin.close();
        fout.close();

        if (found) {
            remove("books.dat");
            rename("temp.dat", "books.dat");
            cout << "Book deleted successfully!\n";
        } else {
            cout << "Book not found!\n";
        }
    }

    void displayIssuedBooks() {
        ifstream fin("issued_books.dat", ios::binary);
        IssuedBook issued;
        cout << "Issued Books:\n";
        while (fin.read((char*)&issued, sizeof(issued))) {
            cout << "User: " << issued.userName << " | Book ID: " << issued.bookId << " | Student Id: " << issued.studentId << endl;
        }
        fin.close();
    }
};

bool librarianLogin() {
    string username, password;
    cout << "Enter Librarian ID: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;
    return (username == "admin" && password == "admin");
}

int main() {
    Library lib;
    IssuedBook issue;
    int choice;

    do {
        cout << "\n1. Librarian\n2. User 1 (Issue Book)\n3. User 2 (Return Book)\n4. Exit\n";
        cout << "Select user role: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                if (librarianLogin()) {
                    int libChoice;
                    do {
                        cout << "\nLibrarian Menu\n1. Add Book\n2. Display Books\n3. Edit Book\n4. Delete Book\n5. View Issued Books\n6. Logout\n";
                        cout << "Enter choice: ";
                        cin >> libChoice;
                        switch (libChoice) {
                            case 1: lib.addBook(); break;
                            case 2: lib.displayBooks(); break;
                            case 3: {
                                int bookId;
                                cout << "Enter Book ID to edit: ";
                                cin >> bookId;
                                lib.editBook(bookId);
                                break;
                            }
                            case 4: {
                                int bookId;
                                cout << "Enter Book ID to delete: ";
                                cin >> bookId;
                                lib.deleteBook(bookId);
                                break;
                            }
                            case 5: lib.displayIssuedBooks(); break;
                            case 6: cout << "Logging out...\n"; break;
                            default: cout << "Invalid choice!\n";
                        }
                    } while (libChoice != 6);
                } else {
                    cout << "Invalid credentials!\n";
                }
                break;
            }
            case 2: issue.issueBook(); break;
            case 3: issue.returnBook(); break;
            case 4: cout << "Exiting...\n"; break;
            default: cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 4);

    return 0;
}
