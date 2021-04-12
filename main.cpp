#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <stack>
#include <map>

using namespace std;

class Tag;

enum class ETagContentType {
    TEXT, TAGS
};

struct BaseTagContent {
    virtual ETagContentType getType() = 0;
    //virtual void addContent(shared_ptr<BaseTagContent>& pContent) = 0;
    virtual void addTag(shared_ptr<Tag>& pTag) = 0;
};

struct TextContent : public BaseTagContent {
    string text;
    virtual ETagContentType getType() override {
        return ETagContentType::TEXT;
    }
    /*
    virtual void addContent(shared_ptr<BaseTagContent>& pContent) override {

    }
    */
    virtual void addTag(shared_ptr<Tag>& pTag) override {}
    void setTextContent(const string& text) {
        this->text = text;
    }
    string& getTextContent() {
        return text;
    }
};

struct TagsContent : public BaseTagContent {
    vector<shared_ptr<Tag>> tags;
    virtual ETagContentType getType() override {
        return ETagContentType::TAGS;
    }
    /*
    virtual void addContent(shared_ptr<BaseTagContent>& pContent) override {
        tags.push_back(static_pointer_cast<BaseTagContent>(pContent));
    }
    */
    vector<shared_ptr<Tag>>& getTreeContent() {
        return tags;
    }
    virtual void addTag(shared_ptr<Tag>& pTag) override {
        tags.push_back(pTag);
    }
};

struct Tag {
    string name;
    shared_ptr<BaseTagContent> pContent;
    Tag(const string& name): name{name} {}
    void setContent(const shared_ptr<BaseTagContent>& pContent) {
        this->pContent = pContent;
    }
    shared_ptr<BaseTagContent> getContent() { 
        return pContent;
    }
};

struct XMLParser {
    enum class ETokenType: char {
      Open  = '<',
      Close = '>',
      OpenHead = '?',
      CloseTag = '/'
    };
    enum class EParseState {
      Start, ReadingTagName, ReadingCloseTagName, ReadingTagProps, ReadingHead, ReadingContent//ReadingText, ReadingTags
    };

    shared_ptr<Tag> root;
    shared_ptr<Tag> curTag;
    string& xmlStr;
    size_t curIdx = 0;
    string curToken;
    EParseState state;
    stack<EParseState> statesStack;
    map<string, shared_ptr<stack<shared_ptr<Tag>>>> tagsMapStack;
    bool wasError;
    string errMsg;
    
    XMLParser(string& xmlStr): xmlStr{xmlStr}, state{EParseState::Start}, wasError{false} {
        
    }
    void read() {
        //cout << "XMLStr: " << curToken << " asf" << "curIdx: " << curIdx << endl;
        if (isEnd() || wasError) return;
        char ch = xmlStr[curIdx];
        ++curIdx;
        maybeSwitchState(ch);
    }
    bool isEnd() {
        return curIdx > xmlStr.size() - 1;
    }
    void maybeSwitchState(char ch) {
        cout << "now: " << endl;
        if (ETokenType::Open == static_cast<ETokenType>(ch)) {
            if (!(state == EParseState::Start || state == EParseState::ReadingContent)) {
                wasError = true;
                errMsg = "Bad state " + string{to_string(static_cast<int>(state))};
                return;
            }
            statesStack.push(state);
            state = EParseState::ReadingTagName;
        } else if (ETokenType::Close == static_cast<ETokenType>(ch)) {
            if (!(state == EParseState::ReadingTagName || state == EParseState::ReadingCloseTagName)) {
                wasError = true;
                errMsg = "Bad state (not true state for reading tag name";
                return;
            }
            if (state == EParseState::ReadingCloseTagName) {
                //auto newTag = make_shared<Tag>(curToken);
                auto itTagsStack = tagsMapStack.find(curToken);
                if (itTagsStack == tagsMapStack.end() || itTagsStack->second.size() == 0) {
                    wasError = true;
                    errMsg = "Not found close tag group: " + string{curToken};
                    return;
                }
                auto lastTag = itTagsStack->second.top();
                itTagsStack->second.pop();


                if (!root) {
                    root = lastTag;
                    curTag = newTag;
                } else {
                    //auto pContent = make_shared<TagsContent
                    curTag->getContent()->addTag(newTag);
                }
            } else if (state == EParseState::ReadingTagName) {
                
                shared_ptr<stack<shared_ptr<Tag>>> pStackTags; // TODO need optimizing (use shared_ptr)
                auto itTagsStack = tagsMapStack.find(curToken);
                if (itTagsStack == tagsMapStack.end()) {
                    pStackTags = make_shared<stack<shared_ptr<Tag>>>();
                } else {
                    pStackTags = itTagsStack->second;
                }
                auto newTag = make_shared<Tag>(curToken);
                pStackTags->push(newTag);
            }
            
            if (statesStack.size() == 0) {
                state = EParseState::Start;
            } else {
                state = statesStack.top();
                statesStack.pop();
            }
        } else {
            if (state == EParseState::ReadingTagProps) {
                //...
            } else if (state == EParseState::ReadingTagName) {
                if (ETokenType::CloseTag == static_cast<ETokenType>(ch)) {
                    state = EParseState::ReadingCloseTagName;
                    return;
                }
                curToken += ch;
            }
        }
    }
};

void handlerTree() {

}

int main() {
    Tag tag("some");
    cout << tag.name;
    cout << endl;
    string xml("<tag>content</tag>");
    XMLParser parser(xml);
    while(!parser.isEnd()) {
        parser.read();
        if (parser.wasError) {
            cerr << "Failed: Malfomed input. Error message: " << parser.errMsg;
            exit(-1);
        }
    }

    cout << endl << "Token: " << parser.curToken;
    return 0;
}
