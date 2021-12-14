#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <algorithm>


using json = nlohmann::json;
using namespace std;

struct Product {
    string name;
    string variety;
    string country;
    string category;
    string harvest;
    double price;
    
};

class Purchase {
    private:
        string code;
        struct tm *date;
        int client_id;
        vector<Product> items;
        double total;
        
        struct tm* GetDate(string date_str) {
            int year, month ,day;
            time_t rawtime;
            struct tm * timeinfo;
            
            replace( date_str.begin(), date_str.end(), '-', ' ' );  
            istringstream( date_str ) >> day >> month >> year;
            
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            timeinfo->tm_year = year - 1900;
            timeinfo->tm_mon = month - 1;
            timeinfo->tm_mday = day;
            mktime ( timeinfo );
            return timeinfo;
        }
        
        vector<Product> GetItems(json items_j) {
            vector<Product> items;
            for (json::iterator it = items_j.begin(); it != items_j.end(); ++it) {
                struct Product product;

                product.name = (*it)["produto"].get<string>();
                product.variety = (*it)["variedade"].get<string>();
                product.country = (*it)["pais"].get<string>();
                product.category = (*it)["categoria"].get<string>();
                product.harvest = (*it)["safra"].get<string>();
                product.price = (*it)["preco"].get<double>();
                
                items.push_back(product);
            }
            
            return items;
        }
        
        int ParseClientId(string id_str) {
            id_str.erase(remove(id_str.begin(), id_str.end(), '.'), id_str.end());
            stringstream intValue(id_str);
            int id = 0;
            intValue >> id;
            
            return id;
        }
    public:
        Purchase(json purchase_data) {
            this->code = purchase_data["codigo"].get<string>();
            this->client_id = ParseClientId(purchase_data["cliente"].get<string>());
            this->total = purchase_data["valorTotal"].get<double>();
            this->date = GetDate(purchase_data["data"].get<string>());
            this->items = GetItems(purchase_data["itens"]);
        }
        
        int GetClientId() {
            return this->client_id;
        }
        
        double GetTotal() {
            return this->total;
        }
        
        int GetYear() {
            return this->date->tm_year + 1900;
        }
        
        struct SortByTotal {
            bool operator() (Purchase const & L, Purchase const & R) { 
                return L.total > R.total; 
            }
        };
        
        bool operator == (const Purchase& p) const { return code == p.code; }
}; 

class Client {
    private:
        int id;
        string name;
        string last_name;
        string cpf;
    public:
        double total_purchases;
    
        Client (int id, string name, string last_name, string cpf) {
            this->id = id;
            this->name = name;
            this->last_name = last_name;
            this->cpf = cpf;
            this->total_purchases = 0;
        }
        
        void AddPurchase(Purchase &new_purchase) {
            this->total_purchases += new_purchase.GetTotal();
        }
        
        int GetId() {
            return this->id;
        }
        
        string GetName() {
            return this->name + " " + this->last_name;
        }
        
        struct SortByTotalPurchases {
            bool operator() (Client const & L, Client const & R) { 
                return L.total_purchases > R.total_purchases; 
            }
        }; 
}; 

void ListClientsByPurchases(vector<Client> client_list);
void GetBiggestPurchase(vector<Purchase> purchase_list, vector<Client> &client_list);

int main()
{
    vector<Client> my_clients;
    vector<Purchase> purchases_list;
    json clients_j, purchases_j;
    ifstream clients_file("clients.json");
    ifstream purchases_file("purchases.json");
    
    
    clients_file >> clients_j;
    purchases_file >> purchases_j;
    
    
    for (json::iterator it = clients_j.begin(); it != clients_j.end(); ++it) {
        Client client(
                    (*it)["id"].get<int>(), 
                    (*it)["nome"].get<string>(),
                    (*it)["sobrenome"].get<string>(),
                    (*it)["cpf"].get<string>()
                    );
        my_clients.push_back(client);
    }
    
    for (json::iterator it = purchases_j.begin(); it != purchases_j.end(); it++) {
        Purchase purchase(*it);
        purchases_list.push_back(purchase);
        for (Client& client: my_clients) {
            int client_id = client.GetId();
            int purchase_id = purchase.GetClientId();
            
            if (client_id == purchase_id) {
                client.AddPurchase(purchase);
            }
        }
        
    } 
    
    ListClientsByPurchases(my_clients);
    GetBiggestPurchase(purchases_list, my_clients);
    
    return 0;
}

void ListClientsByPurchases(vector<Client> client_list) {
    
    sort(client_list.begin(), 
         client_list.end(), 
         Client::SortByTotalPurchases());
    
    cout << "My CLients:" << endl;
    
    for (Client client: client_list) {
        cout << "\tClient: " <<  client.GetName() << endl <<
            "\tPurchases: " << client.total_purchases << endl;
    }
    
}

void GetBiggestPurchase(vector<Purchase> purchase_list, vector<Client> &client_list) {
    Client *client_biggest_purchase_2016;
    
    
    sort(purchase_list.begin(), 
         purchase_list.end(), 
         Purchase::SortByTotal());
         
    for (Purchase purchase: purchase_list) {
        int year_of_purchase = purchase.GetYear();
        if (year_of_purchase != 2016) {
            purchase_list.erase(
                remove(purchase_list.begin(), 
                       purchase_list.end(), purchase), 
                purchase_list.end());
        }
    }
    
    Purchase biggest_purchase_2016(purchase_list[0]);
    int purchase_id = biggest_purchase_2016.GetClientId();
    
    for (Client &client: client_list) {
        int client_id = client.GetId();
        
        if (client_id == purchase_id) {
            client_biggest_purchase_2016 = &client;
            break;
        }
    }
    
    cout << "\nBiggest Purchase of 2016:" << endl;
    cout << "\tClient:" << (*client_biggest_purchase_2016).GetName() << endl;
    cout << "\tPurchase Value: " << biggest_purchase_2016.GetTotal() << endl;
}