#include "Jarvis.hpp"

int main()
{
    Jarvis js;
    js.LoadEtc(ETC);
    js.Run();

 //   Robot rb;
 //   for(;;){
 //       string s;
 //       cout << "请输入:";
 //       cin >> s;
 //       string result;
 //       rb.Talk(s, result);
 //       if(result != ""){
 //           cout << "rebot# " << result << endl;
 //       }else{
 //           cout << "rebot# " << "你说的话我听不懂..." << endl;
 //       }
 //   }

    //string str = "{\"name\": \"zhangsan\", \"lan\": \"c++/Java\", \"age\": 21}";

    //cout << "before->" << str << endl;

    //JSONCPP_STRING errs;
    //Json::Value root;
    //Json::CharReaderBuilder rb;
    //std::unique_ptr<Json::CharReader> const jsonreader(rb.newCharReader());
    //bool res = jsonreader->parse(str.data(), str.data()+str.size(), &root, &errs);
    //if(!res || !errs.empty()){
    //    cerr << "json parse error!" << endl;
    //    return 1;
    //}

    //cout << "after->"<< endl;
    //cout << "name : " << root["name"].asString() << endl;
    //cout << "age  : " << root["age"].asInt() << endl;
    //cout << "lan  : " << root["lan"].asString() << endl;
// 1
//    Json::Value root;
//    Json::StreamWriterBuilder wb;
//    ostringstream os;
//
//    root["name"] = "zhangsan";
//    root["age"]  = 21;
//    root["lan"] = "Java/c++";
//
//    std::unique_ptr<Json::StreamWriter> jw(wb.newStreamWriter());
//    jw->write(root, &os);
//
//    string s = os.str();
//
//    cout << s << endl;

    return 0;
}










