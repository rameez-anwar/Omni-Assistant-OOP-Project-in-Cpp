#include <iostream>
#include <ctime>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include<windows.h>
#include<fstream>

using namespace rapidjson;

using namespace std;




// Callback function to write the response from the API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* response)
{
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}






class VirtualAssistant
 {
public:
    virtual void executeCommand() = 0;
};



class User : public VirtualAssistant
{
private:
    string username;
    string password;

public:
    User(string username, string password) {
        this->username = username;
        this->password = password;
    }

    void executeCommand() override {
        if (login()) {
            time_t currentTime = time(nullptr);
            struct tm* localTime = localtime(&currentTime);

            // Get the hour of the day
            int hour = localTime->tm_hour;

            string greeting;
            if (hour >= 0 && hour < 12) {
                greeting = "Good morning";
            } else if (hour >= 12 && hour < 17) {
                greeting = "Good afternoon";
            } else {
                greeting = "Good evening";
            }

            cout << "\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            cout << "\t\t\t\t\t Virtual Assistant " << endl;
            cout << "\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n" << endl;
            cout << "\t\t\t\t" << greeting << ", " << username << "!" << endl;
            cout << "\t\t\t\t" << asctime(localTime) << endl;

            cout << "\nEnter your command ('weather', 'reminder', 'chat', 'play my favorite song', 'Set Alarm', 'Show Alarm', 'exit'): ";
        } else {
            cout << "Logged in as guest" << endl;
            cout << "\nEnter your command ('weather', 'reminder', 'chat', 'play my favorite song', 'Set Alarm', 'Show Alarm', 'exit'): ";
        }
    }

    bool login() {
        ifstream file("users.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                size_t pos = line.find(':');
                if (pos != string::npos) {
                    string fileUsername = line.substr(0, pos);
                    string filePassword = line.substr(pos + 1);
                    if (fileUsername == username && filePassword == password) {
                        file.close();
                        return true;
                    }
                }
            }
            file.close();
        }
        return false;
    }

    inline void signup() {
        ofstream file("users.txt", ios::app);
        if (file.is_open()) {
            file << username << ":" << password << endl;
            file.close();
            cout << "User registered successfully!" << endl;
        } else {
            cout << "Unable to open file for registration." << endl;
        }
    }
};










// Derived class for Weather
class Weather : public VirtualAssistant {
private:
    string apiKey;
    string location;
    

public:
	
	
	
	
	
    Weather(const string& apiKey) : apiKey(apiKey) {}

    // Getter and setter for location
    string getLocation() const {
        return location;
    }

    void setLocation(const string& location) {
        this->location = location;
    }

    void executeCommand() override {
        // Fetch weather details
        string apiUrl = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + location;
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

            // Response buffer
            string response;

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                Document document;
                document.Parse(response.c_str());

                if (!document.HasParseError()) {
                    if (document.HasMember("error")) {
                        cout << "Error: " << document["error"]["message"].GetString() << endl;
                    }
                    else {
                        const Value& current = document["current"];
                        const Value& location = document["location"];

                        cout << "Location: " << location["name"].GetString() << ", " << location["country"].GetString() << endl;
                        cout << "Local Time: " << location["localtime"].GetString() << endl;
                        cout << "Temperature: " << current["temp_c"].GetDouble() << "°C" << endl;
                        cout << "Condition: " << current["condition"]["text"].GetString() << endl;
                        cout << "Humidity: " << current["humidity"].GetDouble() << "%" << endl;
                        cout << "Wind: " << current["wind_kph"].GetDouble() << " kph" << endl;
                    }
                }
                else {
                    cout << "Failed to parse JSON response." << endl;
                }
            }
            else {
                cout << "Failed to fetch weather details. Error code: " << res << endl;
            }

            // Cleanup
            curl_easy_cleanup(curl);
        }
        else {
            cout << "Failed to initialize CURL." << endl;
        }
    }
};




// Derived class for Chatbot
class ChatAPI;

class ChatBot {
    friend class ChatAPI;

private:
    string apiKey;
    string model;
    Weather* weather;


    // Function to handle the CURL response
    static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
        size_t totalSize = size * nmemb;
        output->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

public:
	
	
	ChatBot(Weather* weatherInfo) 
	{
        weather = weatherInfo;
    }
	
    ChatBot(const string& apiKey, const string& model)
        : apiKey(apiKey), model(model) {}

    void executeCommand() {
        cout << "Welcome to ChatBot. Type 'quit' to exit." << endl;

        while (true) {
            string userInput;
            cout << "User: ";
            getline(cin, userInput);

            if (userInput == "quit") {
                break;
            }

            string response = sendMessage(userInput);
            cout << "ChatBot: " << response << endl;
        }
    }

private:
    string sendMessage(const string& message) {
        string url = "https://api.openai.com/v1/chat/completions";

        CURL* curl = curl_easy_init();
        if (curl) {
            string response;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            // Set the request headers
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            // Set the request data
            string requestData = "{\"messages\": [{\"role\": \"system\", \"content\": \"You are a helpful assistant.\"}, {\"role\": \"user\", \"content\": \"" + message + "\"}], \"model\": \"" + model + "\"}";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());

            // Perform the request
            CURLcode result = curl_easy_perform(curl);

            // Free the headers
            curl_slist_free_all(headers);

            if (result == CURLE_OK) {
                // Parse the JSON response
                rapidjson::Document document;
                document.Parse(response.c_str());

                if (document.IsObject() && document.HasMember("choices") && document["choices"].IsArray() && document["choices"][0].HasMember("message") && document["choices"][0]["message"].HasMember("content")) {
                    string content = document["choices"][0]["message"]["content"].GetString();
                    return content;
                }
                else {
                    cerr << "Failed to get the response. Invalid JSON response received: " << response << endl;
                }
            }
            else {
                cerr << "Failed to get the response. CURL error code: " << result << " (" << curl_easy_strerror(result) << ")" << endl;
            }

            curl_easy_cleanup(curl);
        }
        else {
            cerr << "Failed to initialize CURL." << endl;
        }

        return "";
    }
};

class ChatAPI 
{
public:
    static void executeCommand(ChatBot& chatBot) {
        chatBot.executeCommand();
    }
};

// Derived class for Songs
class Songs : public VirtualAssistant
 {
 	private:
 		string songn;
 		
public:
	Songs()
	{
		songn="";
	}
    void executeCommand() 
	{
         			cout<<"your favourite song is being played \n";
                    ShellExecute(NULL,"open","ijazat.mp3",NULL, NULL, SW_NORMAL);
    }

};



// Derived class for Reminder
class Reminder : public VirtualAssistant 
{
private:
    string apiKey;
    string calendarId;

public:	
	
	inline void printFormattedEvent(const rapidjson::Value& event) {
    if (event.HasMember("summary") && event["summary"].IsString()) {
        cout << "Reminder: " << event["summary"].GetString() << endl;
    }

    if (event.HasMember("start") && event["start"].HasMember("date") && event["start"]["date"].IsString()) {
        cout << "Date: " << event["start"]["date"].GetString() << endl;
    }

    if (event.HasMember("location") && event["location"].IsString()) {
        cout << "Location: " << event["location"].GetString() << endl;
    }

    if (event.HasMember("description") && event["description"].IsString()) {
        cout << "Description: " << event["description"].GetString() << endl;
    }

    cout << endl;
}

	
	
	
	
    Reminder(const string& apiKey, const string& calendarId) : apiKey(apiKey), calendarId(calendarId) {}

    void executeCommand() override {
        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curl = curl_easy_init();

        if (curl) {
            string url = "https://www.googleapis.com/calendar/v3/calendars/" + calendarId + "/events?key=" + apiKey;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            // Disable SSL certificate verification
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            string response;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                rapidjson::Document document;
                document.Parse(response.c_str());

                if (document.HasMember("items") && document["items"].IsArray()) {
                    const rapidjson::Value& items = document["items"];
                    cout << "Reminders List:" << endl;
                    cout << endl;

                    for (rapidjson::SizeType i = 0; i < items.Size(); ++i) {
                        const rapidjson::Value& event = items[i];
                        printFormattedEvent(event);
                    }
                } else {
                    cerr << "No events found in the response." << endl;
                }
            } else {
                cerr << "Failed to fetch calendar events: " << curl_easy_strerror(res) << endl;
            }

            curl_easy_cleanup(curl);
        } else {
            cerr << "Failed to initialize libcurl." << endl;
        }

        curl_global_cleanup();
    }
    
    
    	

    
    
    
    
    
    
};





// Derived class for Alarms
class Alarms : public VirtualAssistant {
private:
    int sethours;
    int setmin;
    string ampm;

public:
    Alarms() {
        this->sethours = 0;
        this->setmin = 0;
        this->ampm = "";
    }

    inline void setAlarm() {
        try {
            cout << "Enter hours: ";
            cin >> sethours;
            if (sethours < 1 || sethours > 12) {
                throw out_of_range("Invalid hour. Please enter a value between 1 and 12.");
            }

            cout << "Enter minutes: ";
            cin >> setmin;
            if (setmin < 0 || setmin > 60) {
                throw out_of_range("Invalid minute. Please enter a value between 0 and 59.");
            }

            cout << "AM or PM: ";
            cin >> ampm;
			if (ampm != "AM" && ampm != "am" && ampm != "Am" && ampm != "PM" && ampm != "pm") {
   			 throw invalid_argument("Invalid AM/PM value. Please enter 'AM' or 'PM'.");
			}


            cout << "Alarm has been set" << endl;
        } catch (const out_of_range& e) {
            cerr << "Error: " << e.what() << endl;
        } catch (const invalid_argument& e) {
            cerr << "Error: " << e.what() << endl;
        } catch (...) {
            cerr << "An unknown error occurred while setting the alarm." << endl;
        }
    }

    void executeCommand() {
        cout << "Alarm set for " << sethours << ":" << setmin << " " << ampm << endl;
    }
};








int main() {
    int choice;
    string username, password;

    while (true) {
        cout << "Menu:" << endl;
        cout << "1. Login" << endl;
        cout << "2. Signup" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        system("cls"); 

        switch (choice) {
            case 1:
                cout << "Enter your username: ";
                getline(cin, username);
                cout << "Enter your password: ";
                getline(cin, password);

                {
                    User user(username, password);
                    user.executeCommand();

                    Weather weather("Weather API here");
                    Reminder reminder("Calender API Here ", "Email Here");
                    ChatBot chatBot("GPT API Here", "gpt-3.5-turbo");
                    Alarms alarms;
                    Songs song;

                    string userInput;

                    while (true) {
                        getline(cin, userInput);

                        if (userInput == "exit") {
                            cout << "Exiting the program. Goodbye!" << endl;
                            return 0;
                        }

                        system("cls"); 

                        if (userInput == "weather") {
                            string location;
   								ChatBot bot(&weather);
                            cout << "Enter location: ";
                            getline(cin, location);
                            weather.setLocation(location);
                            weather.executeCommand();
                        } else if (userInput == "reminder") {
                            reminder.executeCommand();
                        } else if (userInput == "chat") {
                            chatBot.executeCommand();
                        } else if (userInput == "Set Alarm" || userInput == "set alarm") {
                            alarms.setAlarm();
                        } else if (userInput == "Show Alarm" || userInput == "show alarm") {
                            alarms.executeCommand();
                        } else if (userInput == "play my favorite song") {
                            song.executeCommand();
                        } else if (userInput == "open google") {
                            cout << "Starting Google for you \n";
                            system("start https://www.google.com");
                        } else {
                            cout << endl;
                        }

                        cout << "\nEnter your command ('weather', 'reminder', 'chat', 'play my favorite song', 'Set Alarm', 'Show Alarm', 'exit'): ";
                    }
                }

                break;

            case 2:
                cout << "Enter your username: ";
                getline(cin, username);
                cout << "Enter your password: ";
                getline(cin, password);

                {
                    User user(username, password);
                    user.signup();
                }

                break;

            case 3:
                cout << "Exiting the program. Goodbye!" << endl;
                return 0;

            default:
                cout << "Invalid choice. Please try again." << endl;
        }

        system("cls"); 
    }
}