/****************************************************************************
 Copyright (c) 2013 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "UIRichText.h"
#include "platform/CCFileUtils.h"
#include "2d/CCLabel.h"
#include "2d/CCSprite.h"
#include "base/ccUTF8.h"
#include "ui/UIHelper.h"

NS_CC_BEGIN

namespace ui {

    
bool RichElement::init(int tag, const Color3B &color, GLubyte opacity)
{
    _tag = tag;
    _color = color;
    _opacity = opacity;
    return true;
}
    
    
RichElementText* RichElementText::create(int tag, const Color3B &color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize,int outLine,bool underLine)
{
    RichElementText* element = new (std::nothrow) RichElementText();
    if (element && element->init(tag, color, opacity, text, fontName, fontSize,outLine,underLine))
    {
        element->autorelease();
        return element;
    }
    CC_SAFE_DELETE(element);
    return nullptr;
}
    
bool RichElementText::init(int tag, const Color3B &color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize,int outLine,bool underLine)
{
    if (RichElement::init(tag, color, opacity))
    {
        _text = text;
        _fontName = fontName;
        _fontSize = fontSize;
        _outLine = outLine;
        _underLine = underLine;
        return true;
    }
    return false;
}

RichElementImage* RichElementImage::create(int tag, const Color3B &color, GLubyte opacity, const std::string& filePath)
{
    RichElementImage* element = new (std::nothrow) RichElementImage();
    if (element && element->init(tag, color, opacity, filePath))
    {
        element->autorelease();
        return element;
    }
    CC_SAFE_DELETE(element);
    return nullptr;
}
    
bool RichElementImage::init(int tag, const Color3B &color, GLubyte opacity, const std::string& filePath)
{
    if (RichElement::init(tag, color, opacity))
    {
        _filePath = filePath;
        return true;
    }
    return false;
}

RichElementCustomNode* RichElementCustomNode::create(int tag, const Color3B &color, GLubyte opacity, cocos2d::Node *customNode)
{
    RichElementCustomNode* element = new (std::nothrow) RichElementCustomNode();
    if (element && element->init(tag, color, opacity, customNode))
    {
        element->autorelease();
        return element;
    }
    CC_SAFE_DELETE(element);
    return nullptr;
}
    
bool RichElementCustomNode::init(int tag, const Color3B &color, GLubyte opacity, cocos2d::Node *customNode)
{
    if (RichElement::init(tag, color, opacity))
    {
        _customNode = customNode;
        _customNode->retain();
        return true;
    }
    return false;
}
    
RichText::RichText():
_formatTextDirty(true),
_leftSpaceWidth(0.0f),
_verticalSpace(0.0f),
_elementRenderersContainer(nullptr)
{
    
}
    
RichText::~RichText()
{
    _richElements.clear();
}
    
RichText* RichText::create()
{
    RichText* widget = new (std::nothrow) RichText();
    if (widget && widget->init())
    {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}
    
bool RichText::init()
{
    if (Widget::init())
    {
        return true;
    }
    return false;
}
    
void RichText::initRenderer()
{
    _elementRenderersContainer = Node::create();
    _elementRenderersContainer->setAnchorPoint(Vec2(0.5f, 0.5f));
    addProtectedChild(_elementRenderersContainer, 0, -1);
}

void RichText::insertElement(RichElement *element, int index)
{
    _richElements.insert(index, element);
    _formatTextDirty = true;
}
    
void RichText::pushBackElement(RichElement *element)
{
    _richElements.pushBack(element);
    _formatTextDirty = true;
}
    
void RichText::removeElement(int index)
{
    _richElements.erase(index);
    _formatTextDirty = true;
}
    
void RichText::removeElement(RichElement *element)
{
    _richElements.eraseObject(element);
    _formatTextDirty = true;
}
    
void RichText::formatText()
{
    if (_formatTextDirty)
    {
        _elementRenderersContainer->removeAllChildren();
        _elementRenders.clear();
        if (_ignoreSize)
        {
            addNewLine();
            for (ssize_t i=0; i<_richElements.size(); i++)
            {
                RichElement* element = _richElements.at(i);
                Node* elementRenderer = nullptr;
                switch (element->_type)
                {
                    case RichElement::Type::TEXT:
                    {
                        RichElementText* elmtText = static_cast<RichElementText*>(element);
                        if (FileUtils::getInstance()->isFileExist(elmtText->_fontName))
                        {
                            elementRenderer = Label::create();
                            TTFConfig config;
                            config.fontFilePath =elmtText->_fontName;
                            config.fontSize = elmtText->_fontSize;
                            config.outlineSize = elmtText->_outLine;
                            CCLOG("config.outlineSize %d",config.outlineSize);
                            auto labelElement = dynamic_cast<Label*>(elementRenderer);
                            labelElement->setTTFConfig(config);
                            labelElement->setString(elmtText->_text);
                            if(elmtText->_underLine)
                            {
                                auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, elmtText->_fontSize/20));
                                
                                sp->setColor(elmtText->_color);
                                sp->setAnchorPoint(Vec2::ZERO);
                                sp->setPosition(Vec2::ZERO);
                                labelElement->setUserData((void*)sp);
                            }
                        }
                        else
                        {
                            elementRenderer = Label::createWithSystemFont(elmtText->_text.c_str(), elmtText->_fontName, elmtText->_fontSize);
                            if(elmtText->_underLine)
                            {
                                auto labelElement = dynamic_cast<Label*>(elementRenderer);
                                auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, elmtText->_fontSize/20));
                                
                                sp->setColor(elmtText->_color);
                                sp->setAnchorPoint(Vec2::ZERO);
                                sp->setPosition(Vec2::ZERO);
                                labelElement->setUserData((void*)sp);
                            }
                        }
                        break;
                    }
                    case RichElement::Type::IMAGE:
                    {
                        RichElementImage* elmtImage = static_cast<RichElementImage*>(element);
                        elementRenderer = Sprite::create(elmtImage->_filePath.c_str());
                        break;
                    }
                    case RichElement::Type::CUSTOM:
                    {
                        RichElementCustomNode* elmtCustom = static_cast<RichElementCustomNode*>(element);
                        elementRenderer = elmtCustom->_customNode;
                        break;
                    }
                    default:
                        break;
                }
                elementRenderer->setColor(element->_color);
                elementRenderer->setOpacity(element->_opacity);
                pushToContainer(elementRenderer);
            }
        }
        else
        {
            addNewLine();
            for (ssize_t i=0; i<_richElements.size(); i++)
            {
                
                RichElement* element = static_cast<RichElement*>(_richElements.at(i));
                switch (element->_type)
                {
                    case RichElement::Type::TEXT:
                    {
                        RichElementText* elmtText = static_cast<RichElementText*>(element);
                        handleTextRenderer(elmtText->_text.c_str(), elmtText->_fontName.c_str(), elmtText->_fontSize, elmtText->_color, elmtText->_opacity,elmtText->_outLine,elmtText->_underLine);
                        break;
                    }
                    case RichElement::Type::IMAGE:
                    {
                        RichElementImage* elmtImage = static_cast<RichElementImage*>(element);
                        handleImageRenderer(elmtImage->_filePath.c_str(), elmtImage->_color, elmtImage->_opacity);
                        break;
                    }
                    case RichElement::Type::CUSTOM:
                    {
                        RichElementCustomNode* elmtCustom = static_cast<RichElementCustomNode*>(element);
                        handleCustomRenderer(elmtCustom->_customNode);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        formarRenderers();
        _formatTextDirty = false;
    }
}
    
void RichText::handleTextRenderer(const std::string& text, const std::string& fontName, float fontSize, const Color3B &color, GLubyte opacity,int ouline,bool underline)
{
    auto fileExist = FileUtils::getInstance()->isFileExist(fontName);
    Label* textRenderer = nullptr;
    if (fileExist)
    {
//        textRenderer = Label::createWithTTF(text, fontName, fontSize);
        textRenderer = Label::create();
        TTFConfig config;
        config.fontFilePath =fontName;
        config.fontSize = fontSize;
        config.outlineSize = ouline;
        auto labelElement = dynamic_cast<Label*>(textRenderer);
        labelElement->setTTFConfig(config);
        labelElement->setString(text);
        if(underline)
        {
            auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, fontSize/20));
            
            sp->setColor(color);
            sp->setAnchorPoint(Vec2::ZERO);
            sp->setPosition(Vec2::ZERO);
            labelElement->setUserData((void*)sp);
            
        }

    }
    else
    {
        textRenderer = Label::createWithSystemFont(text, fontName, fontSize);
        if(underline)
        {
            auto labelElement = dynamic_cast<Label*>(textRenderer);
            auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, fontSize/20));
            
            sp->setColor(color);
            sp->setAnchorPoint(Vec2::ZERO);
            sp->setPosition(Vec2::ZERO);
            labelElement->setUserData((void*)sp);
        }
    }
    float textRendererWidth = textRenderer->getContentSize().width;
    _leftSpaceWidth -= textRendererWidth;
    if (_leftSpaceWidth < 0.0f)
    {
        float overstepPercent = (-_leftSpaceWidth) / textRendererWidth;
        std::string curText = text;
        size_t stringLength = StringUtils::getCharacterCountInUTF8String(text);
        int leftLength = stringLength * (1.0f - overstepPercent);
        std::string leftWords = Helper::getSubStringOfUTF8String(curText,0,leftLength);
        std::string cutWords = Helper::getSubStringOfUTF8String(curText, leftLength, stringLength - leftLength);
        if (leftLength > 0)
        {
            Label* leftRenderer = nullptr;
            if (fileExist)
            {
//                leftRenderer = Label::createWithTTF(Helper::getSubStringOfUTF8String(leftWords, 0, leftLength), fontName, fontSize);
                leftRenderer = Label::create();
                TTFConfig config;
                config.fontFilePath =fontName;
                config.fontSize = fontSize;
                config.outlineSize = ouline;
                auto labelElement = dynamic_cast<Label*>(leftRenderer);
                labelElement->setTTFConfig(config);
                labelElement->setString(Helper::getSubStringOfUTF8String(leftWords, 0, leftLength));
                if(underline)
                {
                    auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, fontSize/20));
                    
                    sp->setColor(color);
                    sp->setAnchorPoint(Vec2::ZERO);
                    sp->setPosition(Vec2::ZERO);
                    labelElement->setUserData((void*)sp);
                    
                }
            }
            else
            {
                leftRenderer = Label::createWithSystemFont(Helper::getSubStringOfUTF8String(leftWords, 0, leftLength), fontName, fontSize);
                if(underline)
                {
                    auto labelElement = dynamic_cast<Label*>(textRenderer);
                    auto sp = Sprite::createWithTexture(nullptr, Rect(0, 0, labelElement->getBoundingBox().size.width, fontSize/20));
                    
                    sp->setColor(color);
                    sp->setAnchorPoint(Vec2::ZERO);
                    sp->setPosition(Vec2::ZERO);
                    labelElement->setUserData((void*)sp);
                }
            }
            if (leftRenderer)
            {
                leftRenderer->setColor(color);
                leftRenderer->setOpacity(opacity);
                pushToContainer(leftRenderer);
            }
        }

        addNewLine();
        handleTextRenderer(cutWords.c_str(), fontName, fontSize, color, opacity,ouline,underline);
    }
    else
    {
        textRenderer->setColor(color);
        textRenderer->setOpacity(opacity);
        pushToContainer(textRenderer);
    }
}
    
void RichText::handleImageRenderer(const std::string& fileParh, const Color3B &color, GLubyte opacity)
{
    Sprite* imageRenderer = Sprite::create(fileParh);
    handleCustomRenderer(imageRenderer);
}
    
void RichText::handleCustomRenderer(cocos2d::Node *renderer)
{
    Size imgSize = renderer->getContentSize();
    _leftSpaceWidth -= imgSize.width;
    if (_leftSpaceWidth < 0.0f)
    {
        addNewLine();
        pushToContainer(renderer);
        _leftSpaceWidth -= imgSize.width;
    }
    else
    {
        pushToContainer(renderer);
    }
}
    
void RichText::addNewLine()
{
    _leftSpaceWidth = _customSize.width;
    _elementRenders.push_back(new Vector<Node*>());
}
    
void RichText::formarRenderers()
{
//    float setHeight = 0.0f;
    auto pos = this->getPosition();
    if (_ignoreSize)
    {
        float newContentSizeWidth = 0.0f;
        float newContentSizeHeight = 0.0f;
        
        Vector<Node*>* row = (_elementRenders[0]);
        float nextPosX = 0.0f;
        for (ssize_t j=0; j<row->size(); j++)
        {
            Node* l = row->at(j);
            l->setAnchorPoint(Vec2::ZERO);
            l->setPosition(nextPosX, 0.0f);
            _elementRenderersContainer->addChild(l, 1);
            Size iSize = l->getContentSize();
            newContentSizeWidth += iSize.width;
            newContentSizeHeight = MAX(newContentSizeHeight, iSize.height);
            nextPosX += iSize.width;
        }
        _elementRenderersContainer->setContentSize(Size(newContentSizeWidth, newContentSizeHeight));
    }
    else
    {
        float newContentSizeHeight = 0.0f;
        float *maxHeights = new float[_elementRenders.size()];
        
        for (size_t i=0; i<_elementRenders.size(); i++)
        {
            Vector<Node*>* row = (_elementRenders[i]);
            float maxHeight = 0.0f;
            for (ssize_t j=0; j<row->size(); j++)
            {
                Node* l = row->at(j);
                maxHeight = MAX(l->getContentSize().height, maxHeight);
            }
            maxHeights[i] = maxHeight;
            newContentSizeHeight += maxHeights[i];
        }
        /*
         *如果 用户设置 txt:setContentSize(460, 0)，height设置为0，视为自动适应大小的RichText
         *亦可单独弄一个方法用于设置是”否自动设置RichText的高度“，但是，我觉得ContentSize的hight设置为0，就自动设置高度是蛮合理的。与Label的setDimensions用法相似
         */
        if(_customSize.height == 0)
        {
            _customSize.height =newContentSizeHeight;
        }
        float nextPosY = _customSize.height;
        for (size_t i=0; i<_elementRenders.size(); i++)
        {
            Vector<Node*>* row = (_elementRenders[i]);
            float nextPosX = 0.0f;
            nextPosY -= (maxHeights[i] + _verticalSpace);
            
            for (ssize_t j=0; j<row->size(); j++)
            {
                Node* l = row->at(j);
                l->setAnchorPoint(Vec2::ZERO);
                l->setPosition(nextPosX, nextPosY);
                _elementRenderersContainer->addChild(l, 1);
                if (l->getUserData())
                {
                    Sprite* sp = (Sprite*)(l->getUserData());
                    sp->setPosition(Vec2(nextPosX,nextPosY));
                    _elementRenderersContainer->addChild(sp, 1);
                }
                nextPosX += l->getContentSize().width;
            }
        }
        _elementRenderersContainer->setContentSize(_customSize);//这里改成使用_customSize
        delete [] maxHeights;
    }
    
    size_t length = _elementRenders.size();
    for (size_t i = 0; i<length; i++)
	{
        Vector<Node*>* l = _elementRenders[i];
        l->clear();
        delete l;
	}    
    _elementRenders.clear();
    
    if (_ignoreSize)
    {
        Size s = getVirtualRendererSize();
        this->setContentSize(s);
    }
    else
    {
        this->setContentSize(_customSize);
    }
    updateContentSizeWithTextureSize(_contentSize);
    
    _elementRenderersContainer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
}
    
void RichText::adaptRenderers()
{
    this->formatText();
}
    
void RichText::pushToContainer(cocos2d::Node *renderer)
{
    if (_elementRenders.size() <= 0)
    {
        return;
    }
    _elementRenders[_elementRenders.size()-1]->pushBack(renderer);
}
    
void RichText::setVerticalSpace(float space)
{
    _verticalSpace = space;
}
    
void RichText::setAnchorPoint(const Vec2 &pt)
{
    Widget::setAnchorPoint(pt);
    _elementRenderersContainer->setAnchorPoint(pt);
}
    
Size RichText::getVirtualRendererSize() const
{
    return _elementRenderersContainer->getContentSize();
}
    
void RichText::ignoreContentAdaptWithSize(bool ignore)
{
    if (_ignoreSize != ignore)
    {
        _formatTextDirty = true;
        Widget::ignoreContentAdaptWithSize(ignore);
    }
}
    
std::string RichText::getDescription() const
{
    return "RichText";
}

}

NS_CC_END
