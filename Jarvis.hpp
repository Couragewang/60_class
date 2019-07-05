#ifndef _JARVIS_HPP_
#define _JARVIS_HPP_

#include <iostream>
#include <cstdio>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <unistd.h>
#include <pthread.h>
#include <json/json.h>
#include "base/http.h"
#include "speech.h"
using namespace std;

#define BSIZE 128
#define VOICE_FILE "temp_file/voice.wav"
#define VOICE_TTS_FILE "temp_file/tts.mp3"
#define ETC "./command.etc"

class Util{
    private:
        static pthread_t tid;
    public:
        static bool Exec(string command, bool is_print)
        {
            if(!is_print){
                command += ">/dev/null 2>&1";
            }
            FILE *fp = popen(command.c_str(), "r");
            if(NULL == fp){
                cerr << "popen error" << endl;
                return false;
            }
            if(is_print){
                char c;
                while(fread(&c, 1, 1, fp) > 0){
                    cout << c;
                }
            }
            pclose(fp);
            return true;
        }
        static void* Move(void *arg)
        {
            string message = (char*)arg;
            const char *lable = ".....";
            const char *blank = "     ";
            const char *x = "|/-\\";
            int i = 4;
            int j = 0;
            while(1){
                cout << message << "[" << x[j%4] << "]" << lable + i << "\r";
                fflush(stdout);
                i--;
                j++;
                if(i < 0){
                    i = 4;
                    cout << message << "[" << x[j%4] << "]" << blank << "\r";
                }
                usleep(500000);
            }
        }
        static void BeginShowMessage(string message)
        {
            pthread_create(&tid, NULL, Move, (void*)message.c_str());
        }
        static void EndShowMessage()
        {
            pthread_cancel(tid);
        }
};
pthread_t Util::tid;

class Robot{
    private:
        string url = "http://openapi.tuling123.com/openapi/api/v2";
        string api_key = "76637bcc4aec472ea53f3647482bcb29";
        string user_id = "1";
        aip::HttpClient client;
    public:
        Robot()
        {}
        string MakeJsonString(const string &message)
        {
            Json::Value root;
            Json::Value item;
            item["apiKey"] = api_key;
            item["userId"] = user_id;

            root["reqType"] = 0;
            root["userInfo"] = item;

            Json::Value item1;
            item1["text"] = message;
            Json::Value item2;
            item2["inputText"] = item1;
            root["perception"] = item2;
            Json::StreamWriterBuilder wb;
            ostringstream os;
            std::unique_ptr<Json::StreamWriter> jw(wb.newStreamWriter());
            jw->write(root, &os);
            return os.str();
        }
        string RequestPost(string &body)
        {
            string response;
            int code = client.post(url, nullptr, body, nullptr, &response);
            if(code != CURLcode::CURLE_OK){
                return "";
            }

            return response;
        }
        string ParseJson(string &response)
        {
            JSONCPP_STRING errs;
            Json::Value root;
            Json::CharReaderBuilder rb;
            std::unique_ptr<Json::CharReader> const rp(rb.newCharReader());
            bool res = rp->parse(response.data(),\
                    response.data()+response.size(), &root, &errs);
            if(!res || !errs.empty()){
                return "";
            }
            Json::Value item = root["results"][0];
            Json::Value item1 = item["values"];
            return item1["text"].asString();
        }
        void Talk(string message, string &result)
        {
            string body = MakeJsonString(message);
//            cout << body << endl;
            string response = RequestPost(body);
//            cout << response << endl;
            result = ParseJson(response);
        }
        ~Robot()
        {}
};

class SpeechRec{
    private:
        static string app_id;
        static string api_key;
        static string secret_key;
        aip::Speech *client;
    public:
        SpeechRec()
        {
            client = new aip::Speech(app_id, api_key, secret_key);
        }
        string ASR(const string &voice_bin)
        {
            Util::BeginShowMessage("正在识别");
            map<string, string> options;
            options["dev_pid"] = "1536";

            string file_content;
            aip::get_file_content(voice_bin.c_str(), &file_content);

            Json::Value result = client->recognize(file_content, "wav", 16000, options);
            Util::EndShowMessage();
            
            //cout << result.toStyledString() << endl;
            int code = result["err_no"].asInt();
            if(code != 0){
                cerr << "code : " << code << " err_meg: " << result["err_msg"].asString() << endl;
                return "";
            }

            return result["result"][0].asString();
        }
        void TTS(string &text, string voice_tts)
        {
            ofstream ofile;
            string ret;
            map<string, string> options;
            options["spd"] = "5";
            options["per"] = "4";
            options["vol"] = "15";

            Util::BeginShowMessage("正在合成");
            ofile.open(voice_tts, ios::out | ios::binary);
            Json::Value result = client->text2audio(text, options, ret);
            if(ret.empty()){
                cerr << result.toStyledString() << endl;
            }
            else{
                ofile << ret;
            }
            ofile.close();
            Util::EndShowMessage();
        }
        ~SpeechRec()
        {
            delete client;
            client = nullptr;
        }
};
string SpeechRec::app_id = "16296264";
string SpeechRec::api_key = "xNkCCoo4Bt7GGIhGvTlR1BB6";
string SpeechRec::secret_key = "f4BoknKiDUeHmbsHD9ctzBeKHliCtLed";

class Jarvis{
    private:
        SpeechRec sr;
        Robot rt;
        unordered_map<string, string> cmd_map;
    public:
        bool RecordVoice()
        {
            Util::BeginShowMessage("正在录音");
            bool ret = true;
            string command = "arecord -t wav -c 1 -r 16000 -d 3 -f S16_LE ";
            command += VOICE_FILE;

            if(!Util::Exec(command, false)){
                cerr << "Record error!" << endl;
                ret = false;
            }
            Util::EndShowMessage();
            return ret;
        }
    public:
        Jarvis()
        {}
        void LoadEtc(const string &etc)
        {
            ifstream in(etc);
            if(!in.is_open()){
                cerr << "open error!" << endl;
                return;
            }
            string sep = ":";
            char buffer[BSIZE];
            while(in.getline(buffer, sizeof(buffer))){
                string str = buffer;
                size_t pos = str.find(sep);
                if(string::npos == pos){
                    cout << "command etc error!" << endl;
                    continue;
                }
                string k = str.substr(0, pos);
                k+="。";
                string v = str.substr(pos+sep.size());

                //cout << k << endl;
                //cout << v << endl;
                cmd_map.insert(make_pair(k, v));
            }

            in.close();
        }
        bool IsCommand(const string &text, string &out_command)
        {
            auto iter = cmd_map.find(text);
            if(iter != cmd_map.end()){
                out_command = iter->second;
                return true;
            }
            out_command = "";
            return false;
        }
        void PlayVoice(string voice_file)
        {
            string cmd = "cvlc --play-and-exit ";
            cmd += voice_file;
            Util::Exec(cmd, false);
        }
        void Run()
        {
            string voice_bin = VOICE_FILE;
            volatile bool is_quit = false;
            string command;
            while(!is_quit){
                command="";
                if(RecordVoice()){
                    string text = sr.ASR(voice_bin);
                    cout << "我# " << text << endl;
                    if(IsCommand(text, command)){
                        //run command
                        cout << "[hb@bogon Jarvis]$ " << command << endl;
                        Util::Exec(command, true);
                    }else{
                        string message;
                        if(text == "退出。"){
                            message = "欢迎使用，下次再见";
                            is_quit = true;
                        }
                        else{
                            rt.Talk(text, message);
                            cout << "机器人# " << message << endl;
                        }
                        string voice_tts;
                        sr.TTS(message, VOICE_TTS_FILE);
                        PlayVoice(VOICE_TTS_FILE);
                    }
                }
            }
        }
        ~Jarvis()
        {}
};
#endif











