#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct Player{
    std::string name,pos,nation;
    int age=22, ca=100, pa=140, pace=60, pass=60, finish=60, defend=60, physical=60;
    int value=500, wage=20, contractMonths=36, morale=75, fitness=100;
};
struct Club{std::string name,league; int money=50000,reputation=60,pts=0,p=0,w=0,d=0,l=0,gf=0,ga=0; std::vector<Player> squad;};
struct Fixture{int day; std::string comp,home,away; bool played=false; int hg=0,ag=0;};
enum class Screen{Title,NewGame,Home,Squad,PlayerView,Tactics,Schedule,Competitions,Transfers,Scouting,Finance,Match};

static const int W=1440,H=850;
static std::mt19937 rng{std::random_device{}()};
static std::vector<Club> clubs;
static std::vector<Fixture> fixtures;
static int myClub=0, selectedPlayer=0, day=1, seasonYear=2026;
static Screen screen=Screen::Title;
static std::string news="Welcome to DUMOK Football Manager.";
static int formation=0, mentality=1, pressing=1, tempo=1, lineHeight=1, passing=1;
static bool matchRunning=false; static float matchMinute=0; static int matchHG=0,matchAG=0; static float matchClock=0;
static std::vector<std::string> matchEvents;

int RI(int a,int b){std::uniform_int_distribution<int>d(a,b);return d(rng);}
float RF(float a,float b){std::uniform_real_distribution<float>d(a,b);return d(rng);}
bool Btn(Rectangle r,const char* t,Color c={38,70,98,255}){
    Vector2 m=GetMousePosition(); bool h=CheckCollisionPointRec(m,r);
    DrawRectangleRounded(r,.12f,5,h?ColorBrightness(c,.12f):c);
    DrawRectangleRoundedLinesEx(r,.12f,5,2,{9,22,34,255});
    int fs=17; DrawText(t,(int)(r.x+r.width/2-MeasureText(t,fs)/2),(int)(r.y+r.height/2-fs/2),fs,WHITE);
    return h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
void Panel(Rectangle r,Color c={20,37,54,255}){DrawRectangleRounded(r,.025f,4,c);DrawRectangleRoundedLinesEx(r,.025f,4,2,{47,70,89,255});}
std::string DateText(){int month=7+(day-1)/30, dd=1+(day-1)%30; if(month>12){month-=12;} return std::to_string(seasonYear)+"-"+(month<10?"0":"")+std::to_string(month)+"-"+(dd<10?"0":"")+std::to_string(dd);}
Player MakePlayer(std::string pos,int quality){
    static const char* first[]={"Minjun","Jiho","Doyun","Junseo","Hyunwoo","Seungmin","Taeyang","Woojin","Jaeho","Siwoo","Ethan","Lucas","Mateo","Leo","Noah","Liam"};
    static const char* last[]={"Kim","Lee","Park","Choi","Jung","Kang","Han","Yoon","Silva","Garcia","Muller","Rossi","Smith","Martin"};
    Player p; p.name=std::string(first[RI(0,15)])+" "+last[RI(0,13)]; p.pos=pos; p.age=RI(17,32);
    p.ca=std::clamp(quality+RI(-15,15),55,185);
    int room=(p.age<=21?RI(15,55):p.age<=25?RI(5,30):RI(0,12)); p.pa=std::clamp(p.ca+room,60,200);
    auto s=[&](int bias){return std::clamp((int)(p.ca*.48f)+bias+RI(-12,12),20,99);};
    p.pace=s(pos=="ST"||pos=="LW"||pos=="RW"?15:5); p.pass=s(pos=="CM"||pos=="AM"?18:5);
    p.finish=s(pos=="ST"?25:pos=="LW"||pos=="RW"?12:-3); p.defend=s(pos=="CB"||pos=="LB"||pos=="RB"?25:0); p.physical=s(8);
    p.value=std::max(100,(p.ca*p.ca*(34-p.age))/90); p.wage=std::max(5,p.ca*p.ca/800); p.contractMonths=RI(12,60); p.morale=RI(60,90); p.fitness=RI(88,100);
    return p;
}
Club MakeClub(std::string n,std::string l,int q){
    Club c;c.name=n;c.league=l;c.money=RI(25000,110000);c.reputation=q/2;
    const char* pos[]={"GK","GK","LB","CB","CB","RB","DM","CM","CM","AM","LW","RW","ST","ST","CB","CM","LB","RB","RW","ST","CM","CB"};
    for(auto x:pos)c.squad.push_back(MakePlayer(x,q));
    return c;
}
void BuildWorld(){
    clubs.clear(); fixtures.clear();
    std::vector<std::pair<std::string,std::vector<std::string>>> leagues={
      {"England",{"DUMOK FC","London Lions","Manchester Red","Mersey Blue","Northcastle","West Hamlets","Brighton Waves","Villa City"}},
      {"Spain",{"Madrid Blanco","Barcelona Azul","Atletico Capital","Sevilla Rojo","Valencia CF","Bilbao Lions","Villarreal Gold","Real Sociedad"}},
      {"Germany",{"Munich 1900","Dortmund Yellow","Leipzig Bulls","Leverkusen 04","Frankfurt Eagles","Stuttgart","Bremen","Berlin Union"}},
      {"Italy",{"Milano Rosso","Milano Nero","Torino Zebra","Napoli Blue","Roma Wolves","Lazio Sky","Firenze Viola","Atalanta"}},
      {"France",{"Paris Stars","Marseille","Monaco","Lyon","Lille","Nice","Rennes","Lens"}}
    };
    int q=135;
    for(auto &L:leagues){for(auto &n:L.second)clubs.push_back(MakeClub(n,L.first,q+RI(-8,8)));q-=2;}
    myClub=0; day=1; seasonYear=2026;
    // 14 domestic league dates for user's 8-team league (double round-robin simplified)
    std::vector<int> eng; for(int i=0;i<8;i++)eng.push_back(i);
    int fd=8;
    for(int round=0;round<14;round++){
        for(int k=0;k<4;k++){int a=eng[k],b=eng[7-k]; if(round%2){std::swap(a,b);} fixtures.push_back({fd,"Premier Division",clubs[a].name,clubs[b].name});}
        int last=eng.back(); eng.pop_back(); eng.insert(eng.begin()+1,last); fd+=7;
    }
    // Champions League league-phase style sample dates interleaved
    const int uclDays[]={18,32,46,60,74,88,102,116};
    const char* opp[]={"Madrid Blanco","Munich 1900","Milano Nero","Paris Stars","Barcelona Azul","Dortmund Yellow","Napoli Blue","Monaco"};
    for(int i=0;i<8;i++) fixtures.push_back({uclDays[i],"Champions League", i%2?opp[i]:"DUMOK FC", i%2?"DUMOK FC":opp[i]});
    std::sort(fixtures.begin(),fixtures.end(),[](auto&a,auto&b){return a.day<b.day;});
    news="Pre-season complete. The board expects a competitive season.";
}
Club& Me(){return clubs[myClub];}
Fixture* NextFixture(){for(auto &f:fixtures)if(!f.played&&f.day>=day&&(f.home==Me().name||f.away==Me().name))return &f;return nullptr;}
void SimFixture(Fixture &f){
    auto findClub=[&](std::string n)->Club*{for(auto &c:clubs)if(c.name==n)return &c;return nullptr;};
    Club *a=findClub(f.home),*b=findClub(f.away); int qa=a?a->reputation:65,qb=b?b->reputation:65;
    f.hg=std::max(0,(int)std::round(RF(0,2.5f)+(qa-qb)/45.f)); f.ag=std::max(0,(int)std::round(RF(0,2.2f)+(qb-qa)/45.f)); f.played=true;
    if(f.comp=="Premier Division"&&a&&b){a->p++;b->p++;a->gf+=f.hg;a->ga+=f.ag;b->gf+=f.ag;b->ga+=f.hg;if(f.hg>f.ag){a->w++;b->l++;a->pts+=3;}else if(f.hg<f.ag){b->w++;a->l++;b->pts+=3;}else{a->d++;b->d++;a->pts++;b->pts++;}}
}
void Daily(){
    day++;
    for(auto &c:clubs)for(auto &p:c.squad){p.fitness=std::min(100,p.fitness+RI(1,3)); if(day%7==0&&p.age<25&&p.ca<p.pa&&RI(0,100)<18)p.ca++;}
    for(auto &f:fixtures)if(!f.played&&f.day<day)SimFixture(f);
    if(day%15==0){ // AI transfer market activity
        int c=RI(1,(int)clubs.size()-1), pi=RI(0,(int)clubs[c].squad.size()-1); news=clubs[c].name+" are considering offers for "+clubs[c].squad[pi].name+".";
    }
}
void Sidebar(){
    DrawRectangle(0,0,215,H,{12,25,38,255}); DrawText("DUMOK FM",25,25,26,{239,193,75,255});
    DrawText(DateText().c_str(),25,65,15,{154,177,196,255});
    struct I{const char*t;Screen s;}; I it[]={{"HOME",Screen::Home},{"SQUAD",Screen::Squad},{"TACTICS",Screen::Tactics},{"SCHEDULE",Screen::Schedule},{"COMPETITIONS",Screen::Competitions},{"TRANSFERS",Screen::Transfers},{"SCOUTING",Screen::Scouting},{"FINANCE",Screen::Finance}};
    int y=115;for(auto &i:it){if(Btn({18,(float)y,179,45},i.t,screen==i.s?Color{183,137,45,255}:Color{31,57,79,255}))screen=i.s;y+=54;}
    DrawText("Continue",35,H-105,15,{154,177,196,255}); if(Btn({18,H-78,179,48},"NEXT DAY",{47,117,80,255}))Daily();
}
void Header(const char* title){DrawRectangle(215,0,W-215,75,{17,31,45,255});DrawText(title,250,22,28,WHITE);DrawText(Me().name.c_str(),W-300,26,18,{239,193,75,255});}
void DrawTitle(){
    ClearBackground({11,23,36,255});DrawText("DUMOK",95,100,72,{239,193,75,255});DrawText("FOOTBALL MANAGER",98,180,40,WHITE);
    DrawText("CAREER SIMULATION",101,235,18,{140,165,185,255});Panel({90,320,540,350});
    DrawText("Build a career, not just one match.",130,355,23,WHITE);
    if(Btn({135,420,450,65},"NEW CAREER",{192,145,47,255}))screen=Screen::NewGame;
    if(Btn({135,510,450,65},"LOAD CAREER",{42,79,106,255})){BuildWorld();screen=Screen::Home;}
    Panel({710,140,620,530},{17,34,50,255});DrawText("V0.2 CAREER FOUNDATION",750,185,25,{239,193,75,255});
    const char* l[]={"5 playable football nations / 40 clubs","22-player squads with CA / PA / age","Contracts, wages, value, morale, fitness","Calendar-driven league + Champions League","League table and competition schedule","Transfer market + scouting screens","Daily player growth and AI market news","Match day only when a fixture arrives"};
    for(int i=0;i<8;i++)DrawText(TextFormat("- %s",l[i]),760,245+i*48,18,{211,224,234,255});
}
void DrawNew(){
    ClearBackground({11,23,36,255});DrawText("NEW MANAGER CAREER",70,55,35,WHITE);Panel({70,120,1300,620});
    DrawText("Choose your club",110,155,22,{239,193,75,255});
    int idx=0;for(int r=0;r<5;r++){DrawText(clubs[idx].league.c_str(),110,210+r*95,18,{145,172,194,255});for(int k=0;k<8;k++,idx++){Rectangle rr{300.f+k%4*250,190.f+r*95+(k/4)*40,225,34};if(Btn(rr,clubs[idx].name.c_str(),idx==myClub?Color{184,139,48,255}:Color{36,67,91,255}))myClub=idx;}}
    if(Btn({1080,675,245,50},"START CAREER",{45,126,81,255})){day=1;news="You have been appointed manager of "+Me().name+".";screen=Screen::Home;}
}
void DrawHome(){
    ClearBackground({9,20,31,255});Sidebar();Header("CLUB HOME");Fixture*n=NextFixture();
    Panel({245,100,760,205});DrawText("NEXT MATCH",275,125,16,{239,193,75,255});
    if(n){DrawText(n->comp.c_str(),275,160,18,{145,172,194,255});DrawText(n->home.c_str(),275,200,29,WHITE);DrawText("vs",560,205,18,GRAY);DrawText(n->away.c_str(),620,200,29,WHITE);DrawText(TextFormat("Match day: %d  (%s)",n->day,DateText().c_str()),275,250,16,{145,172,194,255});if(n->day<=day&&Btn({815,190,160,60},"MATCH DAY",{190,143,46,255})){matchMinute=0;matchHG=matchAG=0;matchRunning=true;matchEvents.clear();screen=Screen::Match;}}
    Panel({1030,100,380,205});DrawText("CLUB",1060,125,16,{239,193,75,255});DrawText(TextFormat("Budget  EUR %dm",Me().money/1000),1060,165,20,WHITE);DrawText(TextFormat("Squad   %d players",(int)Me().squad.size()),1060,200,18,WHITE);DrawText(TextFormat("League  %d pts",Me().pts),1060,235,18,WHITE);
    Panel({245,330,1165,175});DrawText("INBOX / NEWS",275,355,17,{239,193,75,255});DrawText(news.c_str(),275,400,20,WHITE);
    Panel({245,530,560,260});DrawText("SEASON SNAPSHOT",275,555,17,{239,193,75,255});DrawText(TextFormat("P %d   W %d   D %d   L %d",Me().p,Me().w,Me().d,Me().l),275,605,23,WHITE);DrawText(TextFormat("GF %d   GA %d   GD %+d",Me().gf,Me().ga,Me().gf-Me().ga),275,650,21,WHITE);
    Panel({830,530,580,260});DrawText("QUICK ACTIONS",860,555,17,{239,193,75,255});if(Btn({860,600,235,55},"TRANSFER MARKET"))screen=Screen::Transfers;if(Btn({1120,600,235,55},"TACTICS"))screen=Screen::Tactics;if(Btn({860,680,235,55},"SCHEDULE"))screen=Screen::Schedule;if(Btn({1120,680,235,55},"SQUAD"))screen=Screen::Squad;
}
void DrawSquad(){
    ClearBackground({9,20,31,255});Sidebar();Header("SQUAD");Panel({245,100,800,690});
    DrawText("PLAYER                  POS AGE   CA   PA   VALUE    WAGE  FIT",270,125,15,{145,172,194,255});
    for(int i=0;i<(int)Me().squad.size();i++){auto&p=Me().squad[i];int y=160+i*27;Rectangle rr{260,(float)y-4,760,25};if(i==selectedPlayer)DrawRectangleRec(rr,{42,76,102,255});DrawText(TextFormat("%-22s %-3s %2d   %3d  %3d   %4dk   %3dk  %3d",p.name.c_str(),p.pos.c_str(),p.age,p.ca,p.pa,p.value,p.wage,p.fitness),270,y,14,WHITE);if(CheckCollisionPointRec(GetMousePosition(),rr)&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))selectedPlayer=i;}
    Panel({1070,100,340,690});auto&p=Me().squad[selectedPlayer];DrawText(p.name.c_str(),1100,135,24,WHITE);DrawText(TextFormat("%s | age %d",p.pos.c_str(),p.age),1100,170,17,{239,193,75,255});DrawText(TextFormat("CA %d / PA %d",p.ca,p.pa),1100,215,28,WHITE);DrawText(TextFormat("Value EUR %dk",p.value),1100,265,17,WHITE);DrawText(TextFormat("Wage EUR %dk/w",p.wage),1100,295,17,WHITE);DrawText(TextFormat("Contract %d months",p.contractMonths),1100,325,17,WHITE);
    int vals[]={p.pace,p.pass,p.finish,p.defend,p.physical,p.morale,p.fitness};const char* lab[]={"PACE","PASS","FINISH","DEFEND","PHYSICAL","MORALE","FITNESS"};for(int i=0;i<7;i++){int y=385+i*48;DrawText(lab[i],1100,y,14,{160,184,202,255});DrawRectangle(1180,y,180,14,{34,55,72,255});DrawRectangle(1180,y,(int)(180*vals[i]/100.f),14,{221,177,69,255});DrawText(TextFormat("%d",vals[i]),1365,y,13,WHITE);}
}
void DrawTactics(){
    ClearBackground({9,20,31,255});Sidebar();Header("TACTICS");Panel({245,105,650,685});Rectangle p{285,145,570,605};DrawRectangleRec(p,{38,123,68,255});for(int i=1;i<10;i++)if(i%2==0)DrawRectangle(p.x+i*p.width/10,p.y,p.width/10,p.height,{43,132,73,255});DrawRectangleLinesEx(p,3,RAYWHITE);DrawLine(p.x+p.width/2,p.y,p.x+p.width/2,p.y+p.height,RAYWHITE);
    Panel({925,105,485,685});const char* f[]={"4-3-3","4-2-3-1","4-4-2"};DrawText("FORMATION",960,145,16,GRAY);for(int i=0;i<3;i++)if(Btn({960.f+i*140,175,130,42},f[i],i==formation?Color{184,139,48,255}:Color{36,67,91,255}))formation=i;
    const char* levels[]={"LOW","NORMAL","HIGH"};const char* mental[]={"DEFENSIVE","BALANCED","ATTACKING"};const char* pass[]={"SHORT","MIXED","DIRECT"};
    DrawText("TEAM INSTRUCTIONS",960,260,18,{239,193,75,255});
    struct R{const char*n;int*v;const char**a;};R rows[]={{"Mentality",&mentality,mental},{"Pressing",&pressing,levels},{"Tempo",&tempo,levels},{"Defensive line",&lineHeight,levels},{"Passing",&passing,pass}};
    for(int i=0;i<5;i++){int y=310+i*78;DrawText(rows[i].n,960,y,16,WHITE);DrawText(rows[i].a[*rows[i].v],1120,y,16,{239,193,75,255});if(Btn({1250.f,(float)y-8,120,38},"CHANGE"))*rows[i].v=(*rows[i].v+1)%3;}
    DrawText("These instructions feed the match simulation.",960,715,14,{145,172,194,255});
}
void DrawSchedule(){
    ClearBackground({9,20,31,255});Sidebar();Header("SCHEDULE");Panel({245,105,1165,685});DrawText("DATE        COMPETITION              HOME                    AWAY                    RESULT",275,135,15,{145,172,194,255});int row=0;for(auto&f:fixtures){if(f.home!=Me().name&&f.away!=Me().name)continue;if(row>=20)break;int y=175+row*29;std::string res=f.played?std::to_string(f.hg)+"-"+std::to_string(f.ag):"-";DrawText(TextFormat("Day %-3d     %-22s %-23s %-23s %s",f.day,f.comp.c_str(),f.home.c_str(),f.away.c_str(),res.c_str()),275,y,15,f.day==day?Color{239,193,75,255}:WHITE);row++;}
}
void DrawCompetitions(){
    ClearBackground({9,20,31,255});Sidebar();Header("COMPETITIONS");Panel({245,105,720,685});DrawText("PREMIER DIVISION TABLE",275,135,19,{239,193,75,255});
    std::vector<Club*> t;for(auto&c:clubs)if(c.league==Me().league)t.push_back(&c);std::sort(t.begin(),t.end(),[](auto*a,auto*b){return a->pts>b->pts;});
    DrawText("#  CLUB                     P   W   D   L   GD   PTS",275,180,15,{145,172,194,255});for(int i=0;i<(int)t.size();i++)DrawText(TextFormat("%d  %-24s %2d  %2d  %2d  %2d  %+3d   %2d",i+1,t[i]->name.c_str(),t[i]->p,t[i]->w,t[i]->d,t[i]->l,t[i]->gf-t[i]->ga,t[i]->pts),275,215+i*35,16,t[i]->name==Me().name?Color{239,193,75,255}:WHITE);
    Panel({995,105,415,685});DrawText("CHAMPIONS LEAGUE",1025,135,19,{239,193,75,255});DrawText("League phase",1025,180,16,WHITE);int y=220;for(auto&f:fixtures)if(f.comp=="Champions League"&&(f.home==Me().name||f.away==Me().name)){DrawText(TextFormat("Day %d  %s",f.day,f.played?TextFormat("%d-%d",f.hg,f.ag):"upcoming"),1025,y,15,WHITE);DrawText(TextFormat("%s vs %s",f.home.c_str(),f.away.c_str()),1025,y+22,14,{145,172,194,255});y+=65;}
}
void DrawTransfers(){
    ClearBackground({9,20,31,255});Sidebar();Header("TRANSFER MARKET");Panel({245,105,1165,685});DrawText("GLOBAL PLAYER MARKET",275,135,19,{239,193,75,255});DrawText("PLAYER                  CLUB                 POS AGE   CA   PA*   VALUE",275,175,15,{145,172,194,255});
    int row=0;for(int ci=1;ci<(int)clubs.size()&&row<18;ci++){auto&p=clubs[ci].squad[(ci*3)%clubs[ci].squad.size()];int shownPA=std::clamp(p.pa+RI(-12,12),p.ca,200);int y=210+row*30;DrawText(TextFormat("%-22s %-20s %-3s %2d   %3d  %3d   %4dk",p.name.c_str(),clubs[ci].name.c_str(),p.pos.c_str(),p.age,p.ca,shownPA,p.value),275,y,15,WHITE);if(Btn({1220.f,(float)y-5,145,26},"SCOUT",{42,79,106,255}))news="Scouting report requested for "+p.name+".";row++;}
    DrawText("* PA shown here is an uncertain scouting estimate.",275,760,14,{145,172,194,255});
}
void DrawScouting(){ClearBackground({9,20,31,255});Sidebar();Header("SCOUTING");Panel({245,105,1165,685});DrawText("SCOUTING CENTRE",275,140,21,{239,193,75,255});DrawText("Scout younger players to narrow the uncertainty around potential ability.",275,185,18,WHITE);DrawText("Future version: regions, scout knowledge, assignments and wonderkid reports.",275,235,17,{145,172,194,255});}
void DrawFinance(){ClearBackground({9,20,31,255});Sidebar();Header("FINANCE");Panel({245,105,1165,685});int wage=0;for(auto&p:Me().squad)wage+=p.wage;DrawText("CLUB FINANCES",275,140,21,{239,193,75,255});DrawText(TextFormat("Transfer budget      EUR %d,000",Me().money),275,205,22,WHITE);DrawText(TextFormat("Weekly wage bill     EUR %d,000",wage),275,250,22,WHITE);DrawText("Board objective      Finish in the top half",275,310,18,{145,172,194,255});}
void FinishMatch(){
    Fixture*n=NextFixture(); if(!n)return; n->hg=matchHG;n->ag=matchAG;n->played=true;
    if(n->comp=="Premier Division"){Club* a=nullptr;Club*b=nullptr;for(auto&c:clubs){if(c.name==n->home)a=&c;if(c.name==n->away)b=&c;}if(a&&b){a->p++;b->p++;a->gf+=matchHG;a->ga+=matchAG;b->gf+=matchAG;b->ga+=matchHG;if(matchHG>matchAG){a->w++;b->l++;a->pts+=3;}else if(matchHG<matchAG){b->w++;a->l++;b->pts+=3;}else{a->d++;b->d++;a->pts++;b->pts++;}}}
    news="Full time: "+n->home+" "+std::to_string(matchHG)+"-"+std::to_string(matchAG)+" "+n->away; day=std::max(day,n->day+1);screen=Screen::Home;matchRunning=false;
}
void DrawMatch(){
    ClearBackground({7,18,28,255});Header("MATCH DAY");Fixture*n=NextFixture(); if(!n){screen=Screen::Home;return;}
    Rectangle pitch{45,105,1010,680};DrawRectangleRec(pitch,{35,120,65,255});for(int i=0;i<10;i+=2)DrawRectangle(pitch.x+i*pitch.width/10,pitch.y,pitch.width/10,pitch.height,{42,132,72,255});DrawRectangleLinesEx(pitch,3,RAYWHITE);DrawLine(pitch.x+pitch.width/2,pitch.y,pitch.x+pitch.width/2,pitch.y+pitch.height,RAYWHITE);DrawCircleLines(pitch.x+pitch.width/2,pitch.y+pitch.height/2,75,RAYWHITE);
    // stylized positional players
    for(int team=0;team<2;team++)for(int i=0;i<11;i++){float x=pitch.x+(team?0.72f:0.28f)*pitch.width+(i%4-1.5f)*65;float y=pitch.y+90+(i%6)*95;DrawCircle(x,y,12,team?Color{220,67,70,255}:Color{49,157,221,255});}
    float bx=pitch.x+pitch.width*(.5f+.27f*sinf(matchClock*.7f)),by=pitch.y+pitch.height*(.5f+.35f*cosf(matchClock*.53f));DrawCircle(bx,by,8,WHITE);DrawCircle(bx,by,3,BLACK);
    Panel({1080,105,330,680},{236,232,217,255});DrawText(n->home.c_str(),1110,135,18,{25,39,53,255});DrawText(n->away.c_str(),1110,165,18,{25,39,53,255});DrawText(TextFormat("%d  -  %d",matchHG,matchAG),1160,210,42,{17,31,45,255});DrawText(TextFormat("%02d:%02d",(int)matchMinute,(int)((matchMinute-(int)matchMinute)*60)),1190,265,18,DARKGRAY);
    DrawText("LIVE EVENTS",1110,320,15,DARKGRAY);int y=355;for(int i=std::max(0,(int)matchEvents.size()-8);i<(int)matchEvents.size();i++){DrawText(matchEvents[i].c_str(),1110,y,14,{25,39,53,255});y+=32;}
    DrawText(TextFormat("Formation: %s",formation==0?"4-3-3":formation==1?"4-2-3-1":"4-4-2"),1110,625,15,DARKGRAY);DrawText(TextFormat("Mentality: %s",mentality==0?"Defensive":mentality==1?"Balanced":"Attacking"),1110,650,15,DARKGRAY);
    if(Btn({1110,700,125,45},matchRunning?"PAUSE":"RESUME",{190,143,46,255}))matchRunning=!matchRunning;if(Btn({1250,700,125,45},"TACTICS",{42,79,106,255}))screen=Screen::Tactics;
}
int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT|FLAG_VSYNC_HINT);InitWindow(W,H,"DUMOK Football Manager C++ v0.2W");SetTargetFPS(60);BuildWorld();
    while(!WindowShouldClose()){
        float dt=GetFrameTime();
        if(screen==Screen::Match&&matchRunning){matchClock+=dt;matchMinute+=dt*1.45f;if(RF(0,1)<dt*.055f){bool home=RF(0,1)<(.50f+(mentality-1)*.025f);if(RF(0,1)<.23f){if(home)matchHG++;else matchAG++;matchEvents.push_back(TextFormat("%d' GOAL %s",(int)matchMinute,home?Me().name.c_str():"Opponent"));}else matchEvents.push_back(TextFormat("%d' Chance created",(int)matchMinute));}if(matchMinute>=90)FinishMatch();}
        BeginDrawing();
        switch(screen){case Screen::Title:DrawTitle();break;case Screen::NewGame:DrawNew();break;case Screen::Home:DrawHome();break;case Screen::Squad:DrawSquad();break;case Screen::PlayerView:break;case Screen::Tactics:DrawTactics();break;case Screen::Schedule:DrawSchedule();break;case Screen::Competitions:DrawCompetitions();break;case Screen::Transfers:DrawTransfers();break;case Screen::Scouting:DrawScouting();break;case Screen::Finance:DrawFinance();break;case Screen::Match:DrawMatch();break;}EndDrawing();
    }CloseWindow();return 0;
}