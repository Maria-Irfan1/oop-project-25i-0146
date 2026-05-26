
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

int strToInt(const string& s) {
int result = 0;
for (int i = 0; i < (int)s.length(); i++)
if (s[i] >= '0' && s[i] <= '9')
result = result * 10 + (s[i] - '0');
return result;
}

string intToStr(int n) {

if (n == 0) 
return "0";
string result = "";
bool neg = false;
if (n < 0) 
{ neg = true; n = -n; }
while (n > 0) 
{ result = (char)('0' + n % 10) + result; n /= 10; }
if (neg) 
result = "-" + result;
return result;
}

string padRight(const string& s, int width) {
string r = s;
while ((int)r.length() < width) 
r += " ";
return r;
}

// Extract Nth pipe-delimited field (0-based)
string getField(const string& line, int idx) {

int cur = 0;
size_t start = 0;
while (cur < idx) {
size_t p = line.find('|', start);
if (p == string::npos) return "";
start = p + 1;
cur++;
    }
size_t p = line.find('|', start);
return line.substr(start, p == string::npos ? string::npos : p - start);

}

// ==========================================
// 1. INTERFACES
// ==========================================

class Saveable {
public:
virtual void save(string path) = 0;
virtual ~Saveable() {}
};

// Second base class — used for multiple inheritance with Customer
class Logger {
public:
void logAction(const string& user, const string& action) {

cout << "[LOG] " << user << " -> " << action << endl;

    }
};

// ==========================================
// 2. PIXEL CLASS
// ==========================================

class Pixel {
private:
int r, g, b;
public:
Pixel() : r(0), g(0), b(0) {}
Pixel(int red, int green, int blue)
: r(Pixel::clamp(red)), g(Pixel::clamp(green)), b(Pixel::clamp(blue)) {}

// static Pixel::clamp() utility — required explicitly by spec
static int clamp(int val) {
return (val < 0) ? 0 : (val > 255 ? 255 : val);
    }

int getR() const { return r; }
int getG() const { return g; }
int getB() const { return b; }
void setR(int v) { r = Pixel::clamp(v); }
void setG(int v) { g = Pixel::clamp(v); }
void setB(int v) { b = Pixel::clamp(v); }

// operator+ for blending two pixels
Pixel operator+(const Pixel& o) const {
return Pixel(r + o.r, g + o.g, b + o.b);
    }

// operator<< for ASCII output — spec scale:
// 0=' ' 32='.' 64=':' 96='-' 128='=' 160='+' 192='*' 224='#' 255='@'
friend ostream& operator<<(ostream& os, const Pixel& p) {
int avg = (p.r + p.g + p.b) / 3;
if (avg < 32)  os << ' ';
else if (avg < 64) 
 os << '.';
else if (avg < 96) 
 os << ':';
else if (avg < 128) 
os << '-';
else if (avg < 160) 
    os << '=';
else if (avg < 192)
    os << '+';
else if (avg < 224)
    os << '*';
else if (avg < 255)
    os << '#';
else    
     os << '@';
        return os;
    }
};

// ==========================================
// 3. IMAGE CLASS
// ==========================================

class FilterSession; // forward declaration for friend
 
class Image : public Saveable {
private:
    int width, height;

void allocate(int w, int h) {
width = w; height = h;
grid = new Pixel*[height];
for (int i = 0; i < height; i++)
grid[i] = new Pixel[width];
    }

void deallocate() {
 if (grid) {
for (int i = 0; i < height; i++) delete[] grid[i];
delete[] grid;
grid = nullptr;
        }
    }

public:
    Pixel** grid;

    // FilterSession is a friend — can access grid directly without getters
friend class Filter; 

friend class FilterSession;

Image() : grid(nullptr), width(0), height(0) {}
~Image() { deallocate(); }

// Deep copy constructor
Image(const Image& other) : grid(nullptr), width(0), height(0) {
allocate(other.width, other.height);
for (int y = 0; y < height; y++)
for (int x = 0; x < width; x++)
grid[y][x] = other.grid[y][x];

    }

    // Deep copy assignment operator
Image& operator=(const Image& other) {

if (this == &other) return *this;
deallocate();
allocate(other.width, other.height);
for (int y = 0; y < height; y++)
for (int x = 0; x < width; x++)
grid[y][x] = other.grid[y][x];
return *this;

    }

bool load(const string& path) {
int w, h, c;
unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 3);
if (!data) {
cout << "Load error: " << stbi_failure_reason() << "\n";
return false;
    
}
deallocate();
allocate(w, h);
for (int y = 0; y < h; y++)
for (int x = 0; x < w; x++) {
int idx = (y * w + x) * 3;
grid[y][x] = Pixel(data[idx], data[idx+1], data[idx+2]);
            
}
stbi_image_free(data);
return true;

    }

    // Test pattern generator — required by spec when no real image available
void generateTestPattern() {
deallocate();
allocate(64, 64);
for (int y = 0; y < height; y++)
for (int x = 0; x < width; x++)
grid[y][x] = Pixel(Pixel::clamp(x * 4),Pixel::clamp(y * 4),Pixel::clamp((x + y) * 2));
cout << "Test pattern generated (64x64).\n";
    }

void save(string path) override {

if (width == 0 || height == 0) return;
unsigned char* data = new unsigned char[width * height * 3];
for (int y = 0; y < height; y++)
for (int x = 0; x < width; x++) {
int idx = (y * width + x) * 3;
data[idx] = (unsigned char)grid[y][x].getR();
data[idx+1] = (unsigned char)grid[y][x].getG();
data[idx+2] = (unsigned char)grid[y][x].getB();

}
int ok = stbi_write_png(path.c_str(), width, height, 3, data, width * 3);
if (!ok) cout << "Failed to write PNG!\n";
delete[] data;
    }

int getWidth()  const { return width; }
int getHeight() const { return height; }

// at(row,col) — spec requires this accessor by name
Pixel& at(int row, int col) { return grid[row][col]; }

// getPixel(x,y) convenience wrapper
Pixel& getPixel(int x, int y) { return grid[y][x]; }

void displayASCII() {
if (width == 0) { cout << "No image loaded.\n"; return; }
cout << "\n=== Image Preview (" << width << " x " << height << ") ===\n";
int stepY = (height > 20) ? height / 20 : 1;
int stepX = (width  > 40) ? width  / 40 : 1;
for (int y = 0; y < height; y += stepY) {
for (int x = 0; x < width; x += stepX)
cout << grid[y][x];
cout << "\n";

}
cout << "Image size: " << width << " x " << height << " | Total pixels: " << (width * height) << "\n";
    
 }
};

// ==========================================
// 4. FILE MANAGER CLASSES (one per concern)
// ==========================================

// --- Handles customers.txt ---
class CustomerFileManager {
public:
// CNIC|Password|FullName|Gender|Phone|City|IsBlocked
static void addCustomer(const string& cnic, const string& pass, const string& name, const string& gender,const string& phone, const string& city) {
ofstream fout("customers.txt", ios::app);
if (fout.is_open()) {
fout << cnic << "|" << pass << "|" << name << "|" << gender << "|" << phone << "|" << city << "|0\n";
fout.close();
        }
    }

static bool cnicExists(const string& cnic) {
ifstream fin("customers.txt");
string line;
while (getline(fin, line))
if (!line.empty() && getField(line, 0) == cnic) return true;
return false;
    }

static bool login(const string& cnic, const string& pass, string& outName) {
ifstream fin("customers.txt");
string line;
while (getline(fin, line)) {
if (line.empty()) continue;
if (getField(line, 0) == cnic && getField(line, 1) == pass) {
if (getField(line, 6) == "1") {
cout << "Access Denied: Account is blocked.\n";
return false;

}
outName = getField(line, 2);
return true;
    }
}
return false;
}

static void viewAll() {
ifstream fin("customers.txt");
string line;
cout << "\n--- All Customers ---\n";
cout << padRight("CNIC", 15) << padRight("Name", 15) << padRight("City", 10) << "Blocked\n";
cout << "--------------------------------------------\n";
while (getline(fin, line)) {
if (line.empty()) continue;
cout << padRight(getField(line, 0), 15) << padRight(getField(line, 2), 15)<< padRight(getField(line, 5), 10)<< getField(line, 6) << "\n";
        }
    }

static void searchCustomer(const string& query) {
ifstream fin("customers.txt");
string line;
bool found = false;
cout << "\n--- Search Results ---\n";
cout << padRight("CNIC", 15) << padRight("Name", 15) << padRight("City", 10) << "Blocked\n";
cout << "--------------------------------------------\n";
while (getline(fin, line)) {
if (line.empty()) continue;
string c = getField(line, 0), n = getField(line, 2);
if (c.find(query) != string::npos || n.find(query) != string::npos) {
cout << padRight(c, 15) << padRight(n, 15)<< padRight(getField(line, 5), 10)  << getField(line, 6) << "\n";
found = true;
        }
    }
if (!found) cout << "No results found.\n";
    }

// Block: set IsBlocked=1, also add to blocked_cnics.txt
static void blockCustomer(const string& cnic) {
vector<string> lines;
ifstream fin("customers.txt");
string line;
bool found = false;
while (getline(fin, line)) {
if (!line.empty() && getField(line, 0) == cnic) {
size_t last = line.rfind('|');
line = line.substr(0, last + 1) + "1";
found = true;
    }
lines.push_back(line);}
        
fin.close();
if (!found) { cout << "CNIC not found.\n"; return; }
ofstream fout("customers.txt");
for (int i = 0; i < (int)lines.size(); i++) fout << lines[i] << "\n";
ofstream fblock("blocked_cnics.txt", ios::app);
fblock << cnic << "\n";
cout << "Customer " << cnic << " blocked.\n";
    }

// Delete: remove record entirely (read all, skip matching, rewrite)
static void deleteCustomer(const string& cnic) {
vector<string> lines;
ifstream fin("customers.txt");
string line;
bool found = false;
while (getline(fin, line)) {
if (!line.empty() && getField(line, 0) == cnic)
  found = true;
else
  lines.push_back(line);

}
    
fin.close();
if (!found) { cout << "CNIC not found.\n"; return; }
    ofstream fout("customers.txt");
for (int i = 0; i < (int)lines.size(); i++) fout << lines[i] << "\n";
    cout << "Customer " << cnic << " deleted.\n";
    }
};

// --- Handles catalog.txt ---
class CatalogFileManager {

public:

static void initCatalog() {
ifstream fin("catalog.txt");
if (fin.good()) return;
    fin.close();
ofstream fout("catalog.txt");
fout << "01|Grayscale|Pixel Transform|1\n";
fout << "02|Invert|Pixel Transform|1\n";
fout << "03|Brightness Adjust|Pixel Transform|1\n";
fout << "04|Contrast Stretch|Pixel Transform|1\n";
fout << "05|Red Channel Only|Pixel Transform|1\n";
fout << "06|Green Channel Only|Pixel Transform|1\n";
fout << "07|Blue Channel Only|Pixel Transform|1\n";
fout << "08|Box Blur|Spatial Filter|1\n";
fout << "09|Flip Horizontal|Geometric|1\n";
fout << "10|Flip Vertical|Geometric|1\n";
fout.close();
    }

static bool isEnabled(int filterID) {
ifstream fin("catalog.txt");
string line;
while (getline(fin, line))
if (!line.empty() && strToInt(getField(line, 0)) == filterID)
return getField(line, 3) == "1";
return true;
    
}

static void toggle(int filterID, bool status) {
vector<string> lines;
ifstream fin("catalog.txt");
string line;
while (getline(fin, line)) {
if (!line.empty() && strToInt(getField(line, 0)) == filterID) {
size_t last = line.rfind('|');
line = line.substr(0, last + 1) + (status ? "1" : "0");

}
lines.push_back(line);
}
fin.close();
ofstream fout("catalog.txt");
for (int i = 0; i < (int)lines.size(); i++) fout << lines[i] << "\n";
cout << "Filter " << filterID << " -> " << (status ? "Enabled" : "Disabled") << "\n";

}

static void viewCatalog() {
ifstream fin("catalog.txt");
string line;
cout << "\n--- Filter Catalog ---\n";
cout << padRight("ID", 5) << padRight("Name", 22) << padRight("Category", 18) << "Status\n";
        cout << "---------------------------------------------------\n";
while (getline(fin, line)) {
if (line.empty()) continue;
cout << padRight(getField(line, 0), 5)<< padRight(getField(line, 1), 22) << padRight(getField(line, 2), 18) << (getField(line, 3) == "1" ? "Enabled" : "Disabled") << "\n";
        }
    }
};

// --- Handles sessions.txt ---
class SessionFileManager {
public:
    
static void save(const string& cnic, const string& timestamp, const string& filters, const string& filename) {
ofstream fout("sessions.txt", ios::app);
if (fout.is_open()) {
fout << cnic << "|" << timestamp << "|" << filters << "|" << filename << "\n";
fout.close();
    }
}

static void viewForCNIC(const string& cnic) {
ifstream fin("sessions.txt");
string line;
bool found = false;
cout << "\n--- Session History ---\n";
cout << padRight("Timestamp", 18) << padRight("Filters", 38) << "Output\n";
cout << "-------------------------------------------------------------------\n";
while (getline(fin, line)) {
if (line.empty() || getField(line, 0) != cnic) continue;
cout << padRight(getField(line, 1), 18) << padRight(getField(line, 2), 38) << getField(line, 3) << "\n";
found = true;
 }
if (!found) cout << "No sessions found.\n";

}

static void viewAll() {
ifstream fin("sessions.txt");
string line;
cout << "\n--- All Sessions ---\n";
cout << padRight("CNIC", 15) << padRight("Timestamp", 18) << padRight("Filters", 30) << "Output\n";
cout << "-------------------------------------------------------------------\n";
while (getline(fin, line)) {
if (line.empty()) continue;
cout << padRight(getField(line, 0), 15) << padRight(getField(line, 1), 18) << padRight(getField(line, 2), 30) << getField(line, 3) << "\n";
        }
    }
};

// --- Handles blocked_cnics.txt ---
class BlockedCNICFileManager {
public:
    
static bool isBlocked(const string& cnic) {
ifstream fin("blocked_cnics.txt");
string line;
while (getline(fin, line))
if (line == cnic) return true;
return false;
    }
};

// ==========================================
// 5. FILTER SYSTEM (10 filters)
// ==========================================

class Filter {
public:
    virtual void apply(Image& img) = 0;
    virtual string getName() const = 0;
    virtual ~Filter() {}
};

// --- Pixel Transform Filters ---

class Grayscale : public Filter {
public:

string getName() const override { return "Grayscale"; }
void apply(Image& img) override {
// Direct grid access via friend relationship
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
Pixel& p = img.grid[y][x];
int avg = (p.getR() + p.getG() + p.getB()) / 3;
p.setR(avg); p.setG(avg); p.setB(avg);
    }
}
};

class Invert : public Filter {
public:
string getName() const override { return "Invert"; }
void apply(Image& img) override {
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
Pixel& p = img.grid[y][x];
p.setR(255 - p.getR());
p.setG(255 - p.getG());
p.setB(255 - p.getB());
        }
    }
};

class BrightnessAdjust : public Filter {
private:
    
int level;

public:

BrightnessAdjust(int l) : level(l) {}
string getName() const override {
return "BrightnessAdjust(" + intToStr(level) + ")";
    
}
void apply(Image& img) override {
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
Pixel& p = img.grid[y][x];
p.setR(p.getR() + level);
p.setG(p.getG() + level);
p.setB(p.getB() + level);
}
}

};

class ContrastStretch : public Filter {
public:
string getName() const override { return "ContrastStretch"; }
void apply(Image& img) override {
int w = img.getWidth(), h = img.getHeight();
int minR=255,maxR=0,minG=255,maxG=0,minB=255,maxB=0;
for (int y = 0; y < h; y++)
for (int x = 0; x < w; x++) {
Pixel& p = img.grid[y][x];
if (p.getR()<minR) minR=p.getR(); if (p.getR()>maxR) maxR=p.getR();
if (p.getG()<minG) minG=p.getG(); if (p.getG()>maxG) maxG=p.getG();
if (p.getB()<minB) minB=p.getB(); if (p.getB()>maxB) maxB=p.getB();
            
}
        

for (int y = 0; y < h; y++)
for (int x = 0; x < w; x++) {
Pixel& p = img.grid[y][x];
if (maxR>minR) p.setR((float)(p.getR()-minR)/(maxR-minR)*255);
if (maxG>minG) p.setG((float)(p.getG()-minG)/(maxG-minG)*255);
if (maxB>minB) p.setB((float)(p.getB()-minB)/(maxB-minB)*255);
            
   }
    
  }
};

// Red Channel Only: keep R, set G=0 and B=0
class RedChannelOnly : public Filter {

public:

string getName() const override { return "RedChannelOnly"; }
void apply(Image& img) override {
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
img.grid[y][x].setG(0);
img.grid[y][x].setB(0);
    }
}
};

// Green Channel Only: keep G, set R=0 and B=0
class GreenChannelOnly : public Filter {
public:

string getName() const override { return "GreenChannelOnly"; }
void apply(Image& img) override {
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
img.grid[y][x].setR(0);
img.grid[y][x].setB(0);
    }

  }
};

// Blue Channel Only: keep B, set R=0 and G=0
class BlueChannelOnly : public Filter {
public:
    
string getName() const override { return "BlueChannelOnly"; }
void apply(Image& img) override {
for (int y = 0; y < img.getHeight(); y++)
for (int x = 0; x < img.getWidth(); x++) {
img.grid[y][x].setR(0);
img.grid[y][x].setG(0);
    }
 } 

};

// --- Spatial Filter ---

class BoxBlur : public Filter {
public:

string getName() const override { return "BoxBlur"; }
void apply(Image& img) override {
Image temp(img); // deep copy — friend accesses grid directly
int w = img.getWidth(), h = img.getHeight();
for (int y = 0; y < h; y++)
for (int x = 0; x < w; x++) {
int r=0,g=0,b=0,count=0;
for (int ky=-1; ky<=1; ky++)
for (int kx=-1; kx<=1; kx++) {
int ny=y+ky, nx=x+kx;
if (ny>=0 && ny<h && nx>=0 && nx<w) {
r += temp.grid[ny][nx].getR();
g += temp.grid[ny][nx].getG();
b += temp.grid[ny][nx].getB();
count++;
    }
}
img.grid[y][x].setR(r/count);
img.grid[y][x].setG(g/count);
img.grid[y][x].setB(b/count);
   }
 }
};

// --- Geometric Filters ---

class FlipHorizontal : public Filter {
public:
string getName() const override { return "FlipHorizontal"; }
void apply(Image& img) override {
int w = img.getWidth(), h = img.getHeight();
for (int y = 0; y < h; y++)
for (int x = 0; x < w/2; x++) {
Pixel tmp = img.grid[y][x];
img.grid[y][x]  = img.grid[y][w-1-x];
img.grid[y][w-1-x] = tmp;
     }
  }
};

class FlipVertical : public Filter {
public:
string getName() const override { return "FlipVertical"; }
void apply(Image& img) override {
int w = img.getWidth(), h = img.getHeight();
for (int y = 0; y < h/2; y++)
for (int x = 0; x < w; x++) {
Pixel tmp  = img.grid[y][x];
img.grid[y][x] = img.grid[h-1-y][x];
img.grid[h-1-y][x] = tmp;
    }
   }
};

// ==========================================
// 6. FILTER SESSION
// ==========================================

class FilterSession {
private:
    
vector<Filter*> pipeline;

public:
 // Method chaining — returns FilterSession& so calls can be chained
FilterSession& addFilter(Filter* f) {
pipeline.push_back(f);
return *this;
    }

string getPipelineString() const {
string s = "";
for (int i = 0; i < (int)pipeline.size(); i++)
s += pipeline[i]->getName() + ">";
return s;
    }

bool isEmpty() const { return pipeline.empty(); }

void executePipeline(Image& img) {
for (int i = 0; i < (int)pipeline.size(); i++) {
cout << "Applying " << (i+1) << "/" << pipeline.size() << ": " << pipeline[i]->getName() << "\n";
pipeline[i]->apply(img);
img.displayASCII();
    }
}

void clear() {
for (int i = 0; i < (int)pipeline.size(); i++) delete pipeline[i];
    pipeline.clear();
}

    ~FilterSession() { clear(); }
};

// ==========================================
// 7. USER HIERARCHY
// ==========================================

class User {
protected:

string name, cnic;

public:
User(string n, string c) : name(n), cnic(c) {}
virtual void showDashboard(Image& img) = 0;
virtual ~User() {}

static bool validatePassword(const string& p) {
if ((int)p.length() != 9) return false;
bool hasUpper=false, hasDigit=false;
for (int i=0; i<(int)p.length(); i++) {
if (isupper(p[i])) hasUpper=true;
if (isdigit(p[i])) hasDigit=true;
 }
return hasUpper && hasDigit;
 }

static bool validateCNIC(const string& c) {
if ((int)c.length() != 13) return false;
for (int i=0; i<(int)c.length(); i++)
if (!isdigit(c[i])) return false;
return true;
 
}

string getCNIC() const
 { return cnic; }
string getName() const
 { return name; }

};

// ==========================================
// ADMIN
// ==========================================

class Admin : public User {
private:

static const string ADMIN_CNIC;
static const string ADMIN_PASS;

public:

Admin() : User("System Admin", ADMIN_CNIC) {}

static bool authenticate(const string& c, const string& p) {
 return (c == ADMIN_CNIC && p == ADMIN_PASS);
 
}

bool runPanel(Image& img) {
int choice;
cout << "\n############################################\n";
cout << "#        ADMIN PANEL                        #\n";
cout << "#############################################\n";
cout << "# 1. Manage Filter Catalog (enable/disable) #\n";
cout << "# 2. View All Customers                     #\n";
cout << "# 3. Search Customer (CNIC or Name)         #\n";
cout << "# 4. Block Customer                         #\n";
cout << "# 5. Delete Customer                        #\n";
cout << "# 6. View All Sessions                      #\n";
cout << "# 7. Logout                                 #\n";
cout << "#############################################\n";
cout << "Choice: ";
cin >> choice;

if (choice == 1) {
CatalogFileManager::viewCatalog();
int id, s;
cout << "Filter ID to toggle (1-10): "; cin >> id;
cout << "Set (1=Enable, 0=Disable): "; cin >> s;
CatalogFileManager::toggle(id, s == 1);
        
  }
 else if (choice == 2) { CustomerFileManager::viewAll(); }
else if (choice == 3) {
            
string q; 
cout << "Search query: "; cin >> q;
CustomerFileManager::searchCustomer(q);
    }
else if (choice == 4) {
string c; cout << "CNIC to block: "; cin >> c;
CustomerFileManager::blockCustomer(c);
        
}

else if (choice == 5) {
string c; 
cout << "CNIC to delete: "; cin >> c;
CustomerFileManager::deleteCustomer(c);
       
}
        

else if (choice == 6) { SessionFileManager::viewAll(); }
else if (choice == 7) { cout << "Admin logged out.\n"; return false; }
return true;
    
}

void showDashboard(Image& img) override { runPanel(img); }

};

const string Admin::ADMIN_CNIC = "0000000000000";
const string Admin::ADMIN_PASS = "Admin1234";

// ==========================================
// CUSTOMER (multiple inheritance: User + Logger)
// ==========================================

class Customer : public User, public Logger {
private:

FilterSession* session;
public:

Customer(const string& n, const string& c) : User(n, c), session(new FilterSession()) {}

~Customer() { delete session; }

bool runPanel(Image& img) {
int choice;
cout << "\n############################################\n";
cout << "#  Welcome, " << name << "                         #\n" ;
cout << "############################################\n";
cout << "# 1. Load Image / Test Pattern             #\n";
cout << "# 2. Browse Filter Catalog                 #\n";
cout << "# 3. Add Filter to Pipeline                #\n";
cout << "# 4. Execute Pipeline & Save               #\n";
cout << "# 5. Reset Pipeline                        #\n";
cout << "# 6. View My Session History               #\n";
cout << "# 7. Logout                                #\n";
cout << "############################################\n";
cout << "Choice: ";
cin >> choice;

if (choice == 1) {

cout << "1. Load from JPG/PNG file\n";
cout << "Choice: ";
int c; cin >> c;
            
if (c == 1) {
string path; cout << "Image path: "; cin >> path;
if (img.load(path)) { cout << "Loaded.\n"; img.displayASCII(); }
else { cout << "Load failed. Generating test pattern.\n";
img.generateTestPattern(); 
img.displayASCII(); }//where is the declaration of 
            
}

       
}
        
else if (choice == 2) {
    CatalogFileManager::viewCatalog();
}
else if (choice == 3) {
cout << "\n 1. Grayscale          (ID 01)\n";
cout << " 2. Invert             (ID 02)\n";
cout << " 3. Brightness Adjust  (ID 03)\n";
cout << " 4. Contrast Stretch   (ID 04)\n";
cout << " 5. Red Channel Only   (ID 05)\n";
cout << " 6. Green Channel Only (ID 06)\n";
cout << " 7. Blue Channel Only  (ID 07)\n";
cout << " 8. Box Blur           (ID 08)\n";
cout << " 9. Flip Horizontal    (ID 09)\n";
cout << "10. Flip Vertical      (ID 10)\n";
cout << "Filter number (0 to cancel): ";
int f; cin >> f;
if (f == 0) return true;

 // Check if admin has disabled this filter
if (!CatalogFileManager::isEnabled(f)) {
cout << "That filter is DISABLED by Admin.\n"; return true;
}

if (f == 1) session->addFilter(new Grayscale());
else if (f == 2) session->addFilter(new Invert());
else if (f == 3) {
int v; cout << "Brightness (-100 to 100): ";
 cin >> v;
session->addFilter(new BrightnessAdjust(v));
            }
else if (f == 4) 
 session->addFilter(new ContrastStretch());
else if (f == 5)  
session->addFilter(new RedChannelOnly());
else if (f == 6)  
session->addFilter(new GreenChannelOnly());
else if (f == 7) 
 session->addFilter(new BlueChannelOnly());
else if (f == 8)  
session->addFilter(new BoxBlur());
else if (f == 9)  
session->addFilter(new FlipHorizontal());
else if (f == 10) 
session->addFilter(new FlipVertical());
else { cout << "Invalid filter.\n"; return true; }

cout << "Added. Pipeline: " << session->getPipelineString() << "\n";
    
}
else if (choice == 4) {
if (img.getWidth() == 0) {
    cout << "Load an image first!\n"; return true;
 }
if (session->isEmpty()) {
    cout << "Add at least one filter first!\n"; return true;
}
session->executePipeline(img);

 time_t now = time(0);
char ts[20];
strftime(ts, 20, "%Y%m%d_%H%M%S", localtime(&now));
string timestamp(ts);
string outFile = cnic + "_" + timestamp + ".png";

img.save(outFile);
SessionFileManager::save(cnic, timestamp,session->getPipelineString(), outFile);
logAction(name, "Saved: " + outFile);
cout << "\n[SUCCESS] Saved as: " << outFile << "\n";
session->clear();

}
else if (choice == 5) {
session->clear();
cout << "Pipeline cleared.\n";

}
else if (choice == 6) {
SessionFileManager::viewForCNIC(cnic); 
 
}

else if (choice == 7) {
cout << "Logged out.\n"; return false;

}
return true;

}

void showDashboard(Image& img) override { runPanel(img); }

};

// ==========================================
// 8. MAIN
// ==========================================

void doRegister() {
string cnic, pass, confirm, name, gender, phone, city;
cout << "\n--- REGISTER ---\n";
cout << "CNIC (13 digits): "; cin >> cnic;
if (!User::validateCNIC(cnic)) 
{ cout << "Invalid CNIC.\n"; return; }
if (BlockedCNICFileManager::isBlocked(cnic)) 
{ cout << "CNIC is blocked.\n"; return; }
if (CustomerFileManager::cnicExists(cnic))   
{ cout << "Already registered. Please login.\n"; return; }
cout << "Password (9 chars, 1 uppercase, 1 digit): "; cin >> pass;
if (!User::validatePassword(pass)) 
{ cout << "Password invalid.\n"; return; }
cout << "Confirm Password: "; cin >> confirm;
if (pass != confirm) 
{ cout << "Passwords do not match.\n"; return; }
cin.ignore();
cout << "Full Name: "; 
getline(cin, name);
cout << "Gender (M/F/Other): "; 
cin >> gender;
cout << "Phone: "; 
cin >> phone;
cout << "City: "; 
cin >> city;
CustomerFileManager::addCustomer(cnic, pass, name, gender, phone, city);
cout << "Registered successfully! You can now login.\n";
}

void doCustomerLogin(Image& img) {
string cnic, pass;
int attempts = 0;
while (attempts < 3) {
cout << "\n--- CUSTOMER LOGIN ---\n";
cout << "CNIC: "; cin >> cnic;
cout << "Password: "; cin >> pass;
if (BlockedCNICFileManager::isBlocked(cnic)) { cout << "CNIC is blocked.\n"; return; }
string foundName;
if (CustomerFileManager::login(cnic, pass, foundName)) {
Customer c(foundName, cnic);
while (c.runPanel(img));
return;
    }
attempts++;
cout << "Invalid credentials. Attempts left: " << (3 - attempts) << "\n";
  }
cout << "Too many failed attempts. Returning to main menu.\n";
}

void doAdminLogin(Image& img) {
string cnic, pass;
cout << "\n--- ADMIN LOGIN ---\n";
cout << "CNIC: "; cin >> cnic;
cout << "Password: "; cin >> pass;
if (Admin::authenticate(cnic, pass)) {
Admin adm;
while (adm.runPanel(img));
} else {
cout << "Invalid admin credentials.\n";
  }
}

int main() {
CatalogFileManager::initCatalog(); // create catalog.txt if missing
Image sessionImg;
int choice;
while (true) {
cout << "\n##########################################\n";
cout << "#         IMAGE FILTER STUDIO              #\n";
cout << "############################################\n";
cout << "#  1. Admin Login                          #\n";
cout << "#  2. Customer Login                       #\n";
cout << "#  3. New Customer? Register here          #\n";
cout << "#  4. Exit                                 #\n";
cout << "############################################\n";
cout << "Your choice: ";
cin >> choice;
if (choice == 1) 
doAdminLogin(sessionImg);
else if (choice == 2) 
doCustomerLogin(sessionImg);
else if (choice == 3) 
doRegister();
else if (choice == 4) 
{ cout << "Goodbye!\n"; break; }
else 
cout << "Invalid choice.\n";
    
 }
    return 0;
}
