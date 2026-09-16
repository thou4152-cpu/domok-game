
#include "raylib.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
using namespace std;

struct Player {
    string name, pos;
    int age, ca, pa, wage, value, contract;
};
struct Club {
    string name;
    int rating, budget, pts, gf, ga;
    vector<Player> squad;
};
enum Screen { TITLE, NEW_CAREER, DASHBOARD, SQUAD, TRANSFERS, LEAGUE, SCHEDULE };

static Color BG={12,18,28,255}, PANEL={22,31,45,255}, PANEL2={29,41,58,255};
static Color ACCENT={57,211,147,255}, MUTED={153,167,185,255}, GOLD_C={244,196,86,255};
static vector<Club> clubs;
static int clubIndex=0, day=1, month=7, year=2026, inbox=3;
static Screen screen=TITLE;
static string toast="";
static float toastT=0;

Rectangle R(float x,float y,float w,float h){return {x,y,w,h};}
bool Btn(Rectangle r,const string& s,bool active=false){
    Vector2 m=GetMousePosition(); bool hover=CheckCollisionPointRec(m,r);
    DrawRectangleRounded(r,.18f,8,active?ACCENT:(hover?PANEL2:PANEL));
    DrawRectangleRoundedLinesEx(r,.18f,8,1,active?ACCENT:Color{55,70,89,255});
    int fs=18; int tw=MeasureText(s.c_str(),fs);
    DrawText(s.c_str(),(int)(r.x+(r.width-tw)/2),(int)(r.y+(r.height-fs)/2),fs,active?BG:RAYWHITE);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
void Txt(const string&s,int x,int y,int fs,Color c=RAYWHITE){DrawText(s.c_str(),x,y,fs,c);}
string DateStr(){ return to_string(year)+"."+to_string(month)+"."+to_string(day); }
void AdvanceDay(){
    day++; if(day>30){day=1;month++;} if(month>12){month=1;year++;}
    for(auto &p: clubs[clubIndex].squad){
        if(GetRandomValue(0,99)<6 && p.ca<p.pa) p.ca++;
    }
    toast="날짜가 하루 진행되었습니다"; toastT=2.0f;
}
void Seed(){
    vector<string> names={"Kim Minjun","Lee Junho","Park Siwoo","Choi Hyun","Jung Woojin","Han Jisung","Yoon Taeho","Kang Minseok","Seo Jun","Lim Dohyun","Baek Seung","Song Jae"};
    vector<string> pos={"GK","RB","CB","CB","LB","DM","CM","AM","RW","LW","ST","ST"};
    vector<string> cn={"DUMOK FC","Seoul Phoenix","Busan Mariners","Jeonju Royals","Incheon Blue","Daegu United","Suwon Knights","Daejeon Citizen"};
    for(int c=0;c<8;c++){
        Club cl; cl.name=cn[c]; cl.rating=GetRandomValue(60,82); cl.budget=GetRandomValue(8,45); cl.pts=0;cl.gf=0;cl.ga=0;
        for(int i=0;i<12;i++){
            int pa=GetRandomValue(55,96);
            int ca=GetRandomValue(45, min(pa,82));
            if(GetRandomValue(0,4)==0){ ca=min(pa,GetRandomValue(70,84)); pa=min(100,ca+GetRandomValue(2,10)); }
            cl.squad.push_back({names[i],pos[i],GetRandomValue(18,33),ca,pa,GetRandomValue(8,65),GetRandomValue(1,18),GetRandomValue(1,5)});
        }
        clubs.push_back(cl);
    }
}
void Header(){
    DrawRectangle(0,0,1280,72,Color{15,23,35,255});
    Txt("DUMOK FOOTBALL MANAGER",28,20,26,ACCENT);
    Txt(DateStr(),925,24,18,MUTED);
    Txt("Inbox "+to_string(inbox),1040,24,18,GOLD_C);
    if(Btn(R(1145,15,110,42),"NEXT DAY")) AdvanceDay();
}
void Sidebar(){
    DrawRectangle(0,72,205,648,Color{16,25,38,255});
    Txt(clubs[clubIndex].name,22,95,20,RAYWHITE);
    Txt("Manager",22,122,15,MUTED);
    struct I{const char*n;Screen s;}; I a[]={{"HOME",DASHBOARD},{"SQUAD",SQUAD},{"TRANSFERS",TRANSFERS},{"LEAGUE",LEAGUE},{"SCHEDULE",SCHEDULE}};
    int y=175;
    for(auto &i:a){ if(Btn(R(18,y,169,45),i.n,screen==i.s)) screen=i.s; y+=58; }
}
void Card(Rectangle r,const string&t,const string&v,const string&sub=""){
    DrawRectangleRounded(r,.08f,8,PANEL); DrawRectangleRoundedLinesEx(r,.08f,8,1,Color{48,62,80,255});
    Txt(t,(int)r.x+16,(int)r.y+13,15,MUTED); Txt(v,(int)r.x+16,(int)r.y+39,27,RAYWHITE);
    if(sub!="") Txt(sub,(int)r.x+16,(int)r.y+76,14,MUTED);
}
void Dashboard(){
    Club &c=clubs[clubIndex];
    Txt("MANAGER DASHBOARD",235,100,30,RAYWHITE);
    Card(R(235,150,220,105),"TRANSFER BUDGET","£"+to_string(c.budget)+"M","Board allocation");
    int avg=0;for(auto&p:c.squad)avg+=p.ca;avg/=c.squad.size();
    Card(R(470,150,220,105),"SQUAD ABILITY",to_string(avg),"Average CA");
    Card(R(705,150,220,105),"LEAGUE POINTS",to_string(c.pts),"Season 2026/27");
    Card(R(940,150,280,105),"NEXT MATCH","vs Seoul Phoenix","Friendly · 3 days");
    DrawRectangleRounded(R(235,280,610,370),.04f,8,PANEL);
    Txt("CLUB OVERVIEW",255,300,20,ACCENT);
    Txt("Season objectives",255,342,17,RAYWHITE);
    Txt("• Finish in the top half",270,377,17,MUTED);
    Txt("• Develop young players",270,410,17,MUTED);
    Txt("• Reach cup quarter-final",270,443,17,MUTED);
    Txt("Recent news",255,495,17,RAYWHITE);
    Txt("Scout recommends 3 new players",270,530,16,GOLD_C);
    Txt("Board welcomes the new manager",270,562,16,MUTED);
    Txt("Pre-season training begins",270,594,16,MUTED);
    DrawRectangleRounded(R(865,280,355,370),.04f,8,PANEL);
    Txt("KEY PLAYERS",885,300,20,ACCENT);
    auto s=c.squad; sort(s.begin(),s.end(),[](auto&a,auto&b){return a.ca>b.ca;});
    int y=345; for(int i=0;i<5;i++){Txt(s[i].name,885,y,17,RAYWHITE);Txt(s[i].pos,1040,y,16,MUTED);Txt("CA "+to_string(s[i].ca),1100,y,16,GOLD_C);y+=48;}
}
void Squad(){
    Club &c=clubs[clubIndex]; Txt("FIRST TEAM SQUAD",235,100,30,RAYWHITE);
    Txt("PLAYER",250,155,15,MUTED); Txt("POS",520,155,15,MUTED); Txt("AGE",590,155,15,MUTED); Txt("CA",665,155,15,MUTED); Txt("PA",735,155,15,MUTED); Txt("VALUE",815,155,15,MUTED); Txt("WAGE",930,155,15,MUTED); Txt("CONTRACT",1040,155,15,MUTED);
    int y=185;
    for(auto&p:c.squad){
        DrawRectangleRounded(R(235,y,985,38),.08f,6,(y/38)%2?PANEL:PANEL2);
        Txt(p.name,250,y+10,16,RAYWHITE);Txt(p.pos,520,y+10,16,ACCENT);Txt(to_string(p.age),595,y+10,16,RAYWHITE);
        Txt(to_string(p.ca),670,y+10,16,GOLD_C);Txt(to_string(p.pa),740,y+10,16,MUTED);
        Txt("£"+to_string(p.value)+"M",815,y+10,16,RAYWHITE);Txt("£"+to_string(p.wage)+"k",930,y+10,16,RAYWHITE);Txt(to_string(p.contract)+"y",1055,y+10,16,RAYWHITE); y+=42;
    }
}
void Transfers(){
    Txt("TRANSFER CENTRE",235,100,30,RAYWHITE);
    Txt("Scouted players",235,145,17,MUTED);
    string n[]={"Mateo Silva","Luca Moretti","Noah Jensen","Takumi Arai","Daniel Costa"};
    string p[]={"ST","CM","CB","RW","GK"};
    int ca[]={77,73,71,69,75}, pa[]={91,86,88,94,80}, val[]={24,18,15,12,10};
    int y=190;
    for(int i=0;i<5;i++){
        DrawRectangleRounded(R(235,y,985,70),.06f,8,PANEL);
        Txt(n[i],255,y+14,19,RAYWHITE);Txt(p[i],255,y+41,15,ACCENT);
        Txt("CA "+to_string(ca[i]),500,y+24,17,GOLD_C);Txt("PA "+to_string(pa[i]),590,y+24,17,MUTED);
        Txt("£"+to_string(val[i])+"M",710,y+24,17,RAYWHITE);
        if(Btn(R(1010,y+14,180,42),"MAKE OFFER")){toast="이적 제안을 보냈습니다";toastT=2;}
        y+=84;
    }
}
void League(){
    Txt("LEAGUE TABLE",235,100,30,RAYWHITE);
    vector<pair<int,int>> order; for(int i=0;i<(int)clubs.size();i++) order.push_back({clubs[i].pts,i});
    sort(order.rbegin(),order.rend());
    int y=165, rank=1;
    for(auto &o:order){ auto &c=clubs[o.second]; DrawRectangleRounded(R(235,y,800,48),.05f,6,o.second==clubIndex?Color{35,72,64,255}:PANEL);
        Txt(to_string(rank),255,y+15,16,MUTED);Txt(c.name,300,y+15,17,RAYWHITE);Txt(to_string(c.pts)+" pts",850,y+15,17,GOLD_C); y+=54;rank++;}
}
void Schedule(){
    Txt("FIXTURES & RESULTS",235,100,30,RAYWHITE);
    string opp[]={"Seoul Phoenix","Busan Mariners","Jeonju Royals","Incheon Blue","Daegu United","Suwon Knights"};
    int y=165;
    for(int i=0;i<6;i++){DrawRectangleRounded(R(235,y,900,60),.05f,6,PANEL);Txt("2026.7."+to_string(4+i*5),255,y+20,16,MUTED);Txt(clubs[clubIndex].name+"  vs  "+opp[i],430,y+20,18,RAYWHITE);Txt(i<1?"Friendly":"League",970,y+20,15,ACCENT);y+=70;}
}
void Title(){
    DrawRectangleGradientV(0,0,1280,720,Color{12,24,39,255},Color{5,10,18,255});
    Txt("DUMOK",85,120,70,ACCENT);Txt("FOOTBALL MANAGER",88,195,42,RAYWHITE);
    Txt("Build a club. Shape a dynasty.",92,260,20,MUTED);
    DrawCircle(970,270,150,Color{22,48,45,255});DrawCircleLines(970,270,150,ACCENT);
    DrawCircle(970,270,58,Color{10,18,27,255});DrawText("DM",925,240,55,ACCENT);
    if(Btn(R(90,360,330,58),"NEW CAREER")) screen=NEW_CAREER;
    if(Btn(R(90,435,330,58),"LOAD CAREER")){toast="세이브 시스템은 다음 버전에서 연결";toastT=2;}
    Txt("v0.3 Career Foundation",92,650,15,MUTED);
}
void NewCareer(){
    Txt("CHOOSE YOUR CLUB",65,55,34,RAYWHITE);Txt("Select a club to begin your managerial career",65,100,18,MUTED);
    int x=65,y=155;
    for(int i=0;i<(int)clubs.size();i++){
        Rectangle r=R(x,y,270,150);DrawRectangleRounded(r,.07f,8,PANEL);DrawRectangleRoundedLinesEx(r,.07f,8,1,Color{50,66,84,255});
        DrawCircle(x+45,y+47,24,i==0?ACCENT:GOLD_C);Txt(clubs[i].name,x+80,y+28,19,RAYWHITE);Txt("Rating "+to_string(clubs[i].rating),x+80,y+58,15,MUTED);
        Txt("Budget £"+to_string(clubs[i].budget)+"M",x+20,y+100,16,RAYWHITE);
        if(CheckCollisionPointRec(GetMousePosition(),r)&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){clubIndex=i;screen=DASHBOARD;}
        x+=295;if(x>1000){x=65;y+=180;}
    }
    if(Btn(R(65,640,150,45),"BACK")) screen=TITLE;
}
int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(1280,720,"DUMOK Football Manager v0.3");
    SetTargetFPS(60); Seed();
    while(!WindowShouldClose()){
        BeginDrawing();ClearBackground(BG);
        if(screen==TITLE) Title();
        else if(screen==NEW_CAREER) NewCareer();
        else {Header();Sidebar(); if(screen==DASHBOARD)Dashboard(); else if(screen==SQUAD)Squad(); else if(screen==TRANSFERS)Transfers(); else if(screen==LEAGUE)League(); else Schedule();}
        if(toastT>0){toastT-=GetFrameTime();DrawRectangleRounded(R(455,655,370,42),.3f,8,Color{0,0,0,210});int w=MeasureText(toast.c_str(),16);Txt(toast,640-w/2,668,16,RAYWHITE);}
        EndDrawing();
    }
    CloseWindow(); return 0;
}
