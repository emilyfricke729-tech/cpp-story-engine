﻿#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>

using namespace std;

enum Language { DE, EN };
Language currentLang = DE;

struct Player {
    int familyType    = 0;
    int stress        = 0;
    int aggression    = 0;
    int trauma        = 0;
    int selfworth     = 50;
    int addictionRisk = 0;
};

struct Choice {
    string labelDE, labelEN;
    int nextNode;
    int dStress=0, dAggression=0, dTrauma=0, dSelfworth=0, dAddiction=0;
    int setFamily=0;
};

struct StoryNode {
    string textDE, textEN;
    string logDE, logEN;  
    vector<Choice> choices;
    int nextNode;
    int dStress=0, dAggression=0, dTrauma=0, dSelfworth=0, dAddiction=0;
};

map<int, StoryNode> nodes;

int clampStat(int v){ return max(0, min(100, v)); }

bool loadStoryFromFile(const string& path) {
    ifstream f(path);
    if (!f.is_open()) return false;

    string line;
    int currentId = -1;
    StoryNode* cur = nullptr;


    auto readBlock = [&](ifstream& f) -> string {
        string result, l;
        bool first = true;
        while (getline(f, l)) {
            if (l == "END") break;
            if (!first) result += "\n";
            result += l;
            first = false;
        }
        return result;
    };

    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();

        if (line.substr(0, 5) == "NODE ") {
            currentId = stoi(line.substr(5));
            nodes[currentId] = StoryNode{};
            cur = &nodes[currentId];
        }
        else if (line == "END_NODE") {
            cur = nullptr;
        }
        else if (cur && line.substr(0, 5) == "NEXT ") {
            cur->nextNode = stoi(line.substr(5));
        }
        else if (cur && line.substr(0, 6) == "STATS ") {
            istringstream ss(line.substr(6));
            ss >> cur->dStress >> cur->dAggression
               >> cur->dTrauma >> cur->dSelfworth >> cur->dAddiction;
        }
        else if (cur && line.substr(0, 7) == "LOG_DE ") {
            cur->logDE = line.substr(7);
        }
        else if (cur && line.substr(0, 7) == "LOG_EN ") {
            cur->logEN = line.substr(7);
        }
        else if (cur && line == "DE_TEXT") {
            cur->textDE = readBlock(f);
        }
        else if (cur && line == "EN_TEXT") {
            cur->textEN = readBlock(f);
        }
        else if (cur && line.substr(0, 7) == "CHOICE ") {
            
            Choice c;
            istringstream ss(line.substr(7));
            ss >> c.nextNode >> c.dStress >> c.dAggression
               >> c.dTrauma >> c.dSelfworth >> c.dAddiction >> c.setFamily;

         
            string cl;
            while (getline(f, cl)) {
                if (cl.empty() || cl[0] == '#') continue;
                while (!cl.empty() && (cl.back() == '\r' || cl.back() == ' '))
                    cl.pop_back();
                if (cl == "END_CHOICE") break;
                if (cl.substr(0, 4) == "DE: ") c.labelDE = cl.substr(4);
                if (cl.substr(0, 4) == "EN: ") c.labelEN = cl.substr(4);
            }
            cur->choices.push_back(c);
        }
    }
    return true;
}

void buildStory(){ }

struct GradColor {
    int r,g,b;
    GradColor(int r,int g,int b):r(r),g(g),b(b){}
};

struct GradDef {
    GradColor top, bot;
    GradDef(GradColor t, GradColor b):top(t),bot(b){}
};

GradDef getGrad(int nodeId){
    if(nodeId <= 1)  return {GradColor(10,10,20),GradColor(25,10,35)};
    if(nodeId == 2)  return {GradColor(15,20,40),GradColor(30,15,50)};
    if(nodeId>=10 && nodeId<=19) return {GradColor(28,18,8), GradColor(14,9,4)};
    if(nodeId>=20 && nodeId<=29) return {GradColor(12,12,28),GradColor(22,18,38)};
    if(nodeId>=30 && nodeId<=39) return {GradColor(10,24,14),GradColor(18,32,18)};
    if(nodeId>=40 && nodeId<=49) return {GradColor(30,8,8),  GradColor(14,5,5)};
    if(nodeId>=50 && nodeId<=59) return {GradColor(15,15,30),GradColor(25,20,40)};
    if(nodeId>=60 && nodeId<=69) return {GradColor(20,15,25),GradColor(35,25,40)};
    if(nodeId==71)               return {GradColor(12,12,25),GradColor(20,18,35)};
    if(nodeId>=70 && nodeId<=75) return {GradColor(25,20,14),GradColor(38,28,18)};
    if(nodeId>=82 && nodeId<=86) return {GradColor(18,12,28),GradColor(30,18,42)};
    if(nodeId>=80 && nodeId<=89) return {GradColor(15,20,35),GradColor(25,30,50)};
    return {GradColor(10,10,20),GradColor(20,15,30)};
}

void drawGradientBackground(sf::RenderWindow& window, int nodeId){
    auto g = getGrad(nodeId);
    float W = (float)window.getSize().x;
    float H = (float)window.getSize().y;
    sf::VertexArray va(sf::PrimitiveType::TriangleStrip, 4);
    va[0].position = {0.f, 0.f};
    va[1].position = {W,   0.f};
    va[2].position = {0.f, H  };
    va[3].position = {W,   H  };
    va[0].color = sf::Color(g.top.r, g.top.g, g.top.b);
    va[1].color = sf::Color(g.top.r, g.top.g, g.top.b);
    va[2].color = sf::Color(g.bot.r, g.bot.g, g.bot.b);
    va[3].color = sf::Color(g.bot.r, g.bot.g, g.bot.b);
    window.draw(va);
}

void drawVignette(sf::RenderWindow& window){
    float W = (float)window.getSize().x;
    float H = (float)window.getSize().y;
    const int STEPS = 18;
    for(int i = 0; i < STEPS; ++i){
        float t = (float)i / (float)(STEPS-1);
        float alpha = t*t*160.f;
        float inset = t*60.f;
        sf::RectangleShape r({W - inset*2.f, H - inset*2.f});
        r.setPosition({inset, inset});
        r.setFillColor(sf::Color(0,0,0,0));
        r.setOutlineColor(sf::Color(0,0,0,(uint8_t)alpha));
        r.setOutlineThickness(4.f);
        window.draw(r);
    }
}

void drawPanelBorder(sf::RenderWindow& window, float px, float py, float pw, float ph){
  
    for(int i = 0; i < 3; ++i){
        float e = (float)(i+1)*1.5f;
        uint8_t a = (uint8_t)(60 - i*18);
        sf::RectangleShape r({pw + e*2.f, ph + e*2.f});
        r.setPosition({px - e, py - e});
        r.setFillColor(sf::Color(0,0,0,0));
        r.setOutlineColor(sf::Color(120,90,180,a));
        r.setOutlineThickness(1.f);
        window.draw(r);
    }
    
    sf::RectangleShape border({pw, ph});
    border.setPosition({px, py});
    border.setFillColor(sf::Color(0,0,0,0));
    border.setOutlineColor(sf::Color(100,80,150,180));
    border.setOutlineThickness(1.f);
    window.draw(border);
  
    const float csz = 8.f;
    struct Corner { float x,y,dx,dy; };
    Corner corners[4] = {
        {px,       py,        1.f, 1.f},
        {px+pw,    py,       -1.f, 1.f},
        {px,       py+ph,     1.f,-1.f},
        {px+pw,    py+ph,    -1.f,-1.f}
    };
    sf::Color ca(180,150,220,220);
    for(auto& c : corners){
        sf::RectangleShape h({csz, 2.f});
        h.setPosition({c.x + (c.dx < 0 ? -csz : 0.f), c.y + (c.dy < 0 ? -2.f : 0.f)});
        h.setFillColor(ca);
        window.draw(h);
        sf::RectangleShape v({2.f, csz});
        v.setPosition({c.x + (c.dx < 0 ? -2.f : 0.f), c.y + (c.dy < 0 ? -csz : 0.f)});
        v.setFillColor(ca);
        window.draw(v);
    }
}

void drawStats(sf::RenderWindow& window, const sf::Font& font, const Player& p, float winW){
    
    sf::RectangleShape bg({winW, 70.f});
    bg.setPosition({0.f, 0.f});
    bg.setFillColor(sf::Color(0,0,0,100));
    window.draw(bg);

    const float barW = winW/2.f - 80.f;
    const float barH = 10.f;

    struct BarDef {
        string label;
        int val;
        sf::Color fill, dark;
        float x, y;
    };

    BarDef bars[4] = {
        {"Stress",      p.stress,        {220,70,70},   {100,30,30},  10.f,          8.f},
        {"Selbstwert",  p.selfworth,     {70,200,90},   {30,90,40},   10.f,          34.f},
        {"Suchtgefahr", p.addictionRisk, {220,160,40},  {100,70,20},  winW/2.f+5.f,  8.f},
        {"Trauma",      p.trauma,        {190,80,210},  {90,40,100},  winW/2.f+5.f,  34.f}
    };

    for(auto& b : bars){
      
        sf::Text lbl(font, b.label, 11);
        lbl.setFillColor(sf::Color(200,190,220));
        lbl.setPosition({b.x, b.y});
        window.draw(lbl);

        float ly = b.y + 14.f;

        sf::RectangleShape track({barW, barH});
        track.setPosition({b.x, ly});
        track.setFillColor(sf::Color(30,25,40,200));
        window.draw(track);

        
        float fillW = (float)clampStat(b.val) / 100.f * barW;
        if(fillW > 0.f){
            sf::VertexArray va(sf::PrimitiveType::TriangleStrip, 4);
            va[0].position = {b.x,         ly};
            va[1].position = {b.x+fillW,   ly};
            va[2].position = {b.x,         ly+barH};
            va[3].position = {b.x+fillW,   ly+barH};
            va[0].color = b.fill;
            va[1].color = b.fill;
            va[2].color = b.dark;
            va[3].color = b.dark;
            window.draw(va);
        }

        sf::Text vt(font, to_string(clampStat(b.val)), 11);
        vt.setFillColor(sf::Color(230,220,255));
        vt.setPosition({b.x + barW + 4.f, ly});
        window.draw(vt);
    }


    sf::RectangleShape sep({winW, 1.f});
    sep.setPosition({0.f, 75.f});
    sep.setFillColor(sf::Color(100,80,150,120));
    window.draw(sep);
}

void drawChoices(sf::RenderWindow& window, const sf::Font& font,
                 const vector<Choice>& choices, int hovered,
                 float startY, Language lang){
    for(int i = 0; i < (int)choices.size(); ++i){
        const string& txt = (lang == DE) ? choices[i].labelDE : choices[i].labelEN;
        bool isHov = (i == hovered);
        float cy = startY + (float)i * 26.f;

        if(isHov){
        
            sf::RectangleShape hi({500.f, 22.f});
            hi.setPosition({10.f, cy - 2.f});
            hi.setFillColor(sf::Color(100,80,160,60));
            window.draw(hi);
        
            sf::RectangleShape accent({3.f, 22.f});
            accent.setPosition({10.f, cy - 2.f});
            accent.setFillColor(sf::Color(180,150,255,220));
            window.draw(accent);
        }

        sf::Text t(font, txt, 15);
        t.setFillColor(isHov ? sf::Color(255,235,120) : sf::Color(185,175,205));
        t.setPosition({22.f, cy});
        window.draw(t);
    }
}


void drawLog(sf::RenderWindow& window, const sf::Font& font,
             const vector<string>& log, Language lang,
             unsigned winW, unsigned winH) {

    
    sf::RectangleShape overlay({(float)winW, (float)winH});
    overlay.setFillColor(sf::Color(5, 3, 12, 230));
    window.draw(overlay);

    
    sf::RectangleShape box({(float)winW - 40.f, (float)winH - 60.f});
    box.setPosition({20.f, 30.f});
    box.setFillColor(sf::Color(12, 8, 22, 240));
    box.setOutlineColor(sf::Color(120, 90, 180, 200));
    box.setOutlineThickness(1.5f);
    window.draw(box);


    string title = (lang == DE) ? "~ Dein Tagebuch ~" : "~ Your Journal ~";
    sf::Text titleT(font, title, 17);
    titleT.setFillColor(sf::Color(200, 170, 255));
    titleT.setPosition({30.f, 36.f});
    window.draw(titleT);

    
    sf::RectangleShape sep({(float)winW - 40.f, 1.f});
    sep.setPosition({20.f, 58.f});
    sep.setFillColor(sf::Color(100, 80, 150, 150));
    window.draw(sep);

    if (log.empty()) {
        string empty = (lang == DE)
            ? "Noch keine Eintraege."
            : "No entries yet.";
        sf::Text emptyT(font, empty, 14);
        emptyT.setFillColor(sf::Color(120, 110, 140));
        emptyT.setPosition({30.f, 70.f});
        window.draw(emptyT);
    } else {
        float y = 66.f;
       
        int start = (int)log.size() - 1;
        int shown = 0;
        for (int i = start; i >= 0 && y < (float)winH - 50.f; --i) {
            
            string prefix = to_string(i + 1) + ". ";
            sf::Text num(font, prefix, 12);
            num.setFillColor(sf::Color(140, 110, 180, 180));
            num.setPosition({28.f, y});
            window.draw(num);

            sf::Text entry(font, log[i], 13);
            entry.setFillColor(sf::Color(210, 200, 230));
            entry.setPosition({52.f, y});
            window.draw(entry);
            y += 22.f;
            shown++;
        }
    }


    string hint = (lang == DE) ? "[ L ] Schliessen" : "[ L ] Close";
    sf::Text hintT(font, hint, 12);
    hintT.setFillColor(sf::Color(100, 85, 130, 200));
    hintT.setPosition({20.f, (float)winH - 24.f});
    window.draw(hintT);
}

int main(){
  
    if (!loadStoryFromFile("story.txt")) {
        cout << "story.txt nicht gefunden - nutze eingebaute Story." << endl;
        buildStory();
    } else {
        cout << "story.txt geladen: " << nodes.size() << " Nodes." << endl;
    }

    const unsigned int winW = 560;
    const unsigned int winH = 420;

    sf::RenderWindow window(
        sf::VideoMode({winW, winH}),
        "Sucht & Schicksal",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(
        (int)(desktop.size.x / 2 - winW / 2),
        (int)(desktop.size.y / 2 - winH / 2)
    ));

    sf::Font font;
    if(!font.openFromFile("arial.ttf"))
        cout << "Fehler: arial.ttf nicht gefunden!" << endl;

    Player player;
    int currentNode   = 0;
    int hoveredChoice = 0;

    vector<string> gameLog;  
    bool showLog = false;    

    string fullText, displayedText;
    int   twIndex = 0;
    float twTimer = 0.f;
    bool  twDone  = false;
    const float TW_SPEED = 0.018f;

    float fadeAlpha  = 255.f;
    bool  fadingIn   = true;
    bool  fadingOut  = false;
    int   fadeTarget = -1;
    const float FADE_SPEED = 150.f;

   
    auto resolveNode = [&](int id) -> int {
        if(id == 62){
            if(player.familyType == 1) return 70;
            if(player.familyType == 2) return 71;
            if(player.familyType == 3) return 73;
            if(player.familyType == 4) return 74;
            return 70;
        }
        if(id == 76){
            if(player.familyType == 2) return 82;
            return (player.selfworth < 40) ? 80 : 81;
        }
        return id;
    };

    auto loadNode = [&](int id){
        if(nodes.find(id) == nodes.end()) return;
        auto& n = nodes[id];
        
        player.stress        = clampStat(player.stress        + n.dStress);
        player.aggression    = clampStat(player.aggression    + n.dAggression);
        player.trauma        = clampStat(player.trauma        + n.dTrauma);
        player.selfworth     = clampStat(player.selfworth     + n.dSelfworth);
        player.addictionRisk = clampStat(player.addictionRisk + n.dAddiction);

       
        const string& logEntry = (currentLang == DE) ? n.logDE : n.logEN;
        if (!logEntry.empty())
            gameLog.push_back(logEntry);

        currentNode   = id;
        hoveredChoice = 0;
        const string& raw = (currentLang == DE) ? n.textDE : n.textEN;
        fullText      = raw;
        displayedText = "";
        twIndex = 0; twTimer = 0.f; twDone = false;
    };

    auto goTo = [&](int id){
        fadingOut  = true;
        fadeTarget = id;
    };

    loadNode(0);

    sf::Clock clock;

    while(window.isOpen()){
        float dt = clock.restart().asSeconds();

        while(const auto event = window.pollEvent()){
            if(event->is<sf::Event::Closed>()) window.close();

            if(const auto* key = event->getIf<sf::Event::KeyPressed>()){
                
                if(key->code == sf::Keyboard::Key::L){
                    showLog = !showLog;
                    continue;
                }
               
                if(showLog) continue;

               
                if(!twDone){
                    displayedText = fullText;
                    twIndex = (int)fullText.size();
                    twDone  = true;
                    continue;
                }

                if(nodes.find(currentNode) == nodes.end()) continue;
                auto& n = nodes[currentNode];

                if(!n.choices.empty()){
                    if(key->code == sf::Keyboard::Key::Up)
                        hoveredChoice = (hoveredChoice - 1 + (int)n.choices.size()) % (int)n.choices.size();
                    if(key->code == sf::Keyboard::Key::Down)
                        hoveredChoice = (hoveredChoice + 1) % (int)n.choices.size();

                    sf::Keyboard::Key numKeys[4] = {
                        sf::Keyboard::Key::Num1, sf::Keyboard::Key::Num2,
                        sf::Keyboard::Key::Num3, sf::Keyboard::Key::Num4
                    };
                    for(int i = 0; i < 4; ++i)
                        if(key->code == numKeys[i])
                            hoveredChoice = min(i, (int)n.choices.size()-1);

                    if(key->code == sf::Keyboard::Key::Enter ||
                       key->code == sf::Keyboard::Key::Space){

                        const Choice& c = n.choices[hoveredChoice];

                       
                        if(currentNode == 0)
                            currentLang = (hoveredChoice == 0) ? DE : EN;

                       
                        player.stress        = clampStat(player.stress        + c.dStress);
                        player.aggression    = clampStat(player.aggression    + c.dAggression);
                        player.trauma        = clampStat(player.trauma        + c.dTrauma);
                        player.selfworth     = clampStat(player.selfworth     + c.dSelfworth);
                        player.addictionRisk = clampStat(player.addictionRisk + c.dAddiction);
                        if(c.setFamily != 0) player.familyType = c.setFamily;

                        int next = resolveNode(c.nextNode);
                        goTo(next);
                    }
                } else {
                    if(key->code == sf::Keyboard::Key::Enter ||
                       key->code == sf::Keyboard::Key::Space){
                        if(n.nextNode < 0){
                            window.close();
                        } else {
                            int next = resolveNode(n.nextNode);
                            goTo(next);
                        }
                    }
                }
            }
        }

      
        if(!twDone){
            twTimer += dt;
            while(twTimer >= TW_SPEED && twIndex < (int)fullText.size()){
                twTimer -= TW_SPEED;
                displayedText += fullText[twIndex++];
            }
            if(twIndex >= (int)fullText.size()) twDone = true;
        }

       
        if(fadingOut){
            fadeAlpha += FADE_SPEED * dt;
            if(fadeAlpha >= 255.f){
                fadeAlpha = 255.f;
                fadingOut = false;
                fadingIn  = true;
                loadNode(fadeTarget);
            }
        }
        if(fadingIn){
            fadeAlpha -= FADE_SPEED * dt;
            if(fadeAlpha <= 0.f){ fadeAlpha = 0.f; fadingIn = false; }
        }

       
        window.clear(sf::Color(5,5,10));

        drawGradientBackground(window, currentNode);
        drawVignette(window);

        if(currentNode >= 50)
            drawStats(window, font, player, (float)winW);

        float panelY = (currentNode >= 50) ? 82.f : 10.f;
        float panelW = (float)winW - 20.f;
        float panelH = (float)winH - panelY - 28.f;

      
        sf::RectangleShape panel({panelW, panelH});
        panel.setPosition({10.f, panelY});
        panel.setFillColor(sf::Color(8,5,15,175));
        window.draw(panel);

       
        drawPanelBorder(window, 10.f, panelY, panelW, panelH);

        
        sf::Text mainText(font, displayedText, 16);
        mainText.setFillColor(sf::Color(235,225,255));
        mainText.setPosition({18.f, panelY + 10.f});
        window.draw(mainText);

        if(nodes.find(currentNode) != nodes.end()){
            auto& cn = nodes[currentNode];

            if(twDone && !cn.choices.empty()){
              
                int lineCount = 0;
                for(char c : displayedText) if(c == '\n') lineCount++;
                float textH = (float)(lineCount + 1) * 20.f;
                float choiceY = panelY + 12.f + textH + 8.f;
                float maxChoiceY = (float)winH - 35.f - (float)cn.choices.size() * 26.f;
                if(choiceY > maxChoiceY) choiceY = maxChoiceY;
                drawChoices(window, font, cn.choices, hoveredChoice, choiceY, currentLang);
            }

            if(twDone){
                string hint;
                if(!cn.choices.empty()){
                    hint = (currentLang == DE)
                        ? "[ Pfeiltasten / 1-4 ] Waehlen   [ ENTER ] Bestaetigen"
                        : "[ Arrow keys / 1-4 ] Select   [ ENTER ] Confirm";
                } else {
                    if(cn.nextNode < 0)
                        hint = (currentLang == DE) ? "[ ENTER ] Beenden" : "[ ENTER ] Quit";
                    else
                        hint = (currentLang == DE) ? "[ ENTER ] Weiter" : "[ ENTER ] Continue";
                }
                sf::Text hintT(font, hint, 12);
                hintT.setFillColor(sf::Color(120,100,160,200));
                hintT.setPosition({10.f, (float)winH - 20.f});
                window.draw(hintT);
            }
        }

    
        if(fadeAlpha > 0.f){
            sf::RectangleShape ov({(float)winW, (float)winH});
            ov.setFillColor(sf::Color(0,0,0,(uint8_t)fadeAlpha));
            window.draw(ov);
        }

        
        if(showLog)
            drawLog(window, font, gameLog, currentLang, winW, winH);

       
        if(!showLog && currentNode >= 2){
            sf::Text logHint(font, "[ L ] Journal", 11);
            logHint.setFillColor(sf::Color(80, 65, 110, 160));
            logHint.setPosition({(float)winW - 90.f, (float)winH - 18.f});
            window.draw(logHint);
        }

        window.display();
    }

    return 0;
}