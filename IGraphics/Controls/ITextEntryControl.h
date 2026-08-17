/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
 */

#pragma once

/**
 * @file
 * @ingroup SpecialControls
 * @brief A Text entry widget drawn by IGraphics to optionally override platform text entries.
 * This allows better matching of text rendering during text input and display, but has disadvantages in terms of accessibility.
 * This code is largely based on VSTGUI's generictextedit, using stb_textedit
 */

#define STB_TEXTEDIT_CHARTYPE char16_t
#define STB_TEXTEDIT_POSITIONTYPE int
#define STB_TEXTEDIT_STRING iplug::igraphics::ITextEntryControl
#define STB_TEXTEDIT_KEYTYPE uint32_t

#include "stb_textedit.h"

#include "IControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A Text entry widget drawn by IGraphics.
 * This is a special control that lives outside the main IGraphics control stack.
 * It can be added with IGraphics::AttachTextEntryControl().
 * It should not be used in the main control stack.
 * @ingroup SpecialControls */
class ITextEntryControl : public IControl
{
public:
  ITextEntryControl();
  
  //IControl
  void Draw(IGraphics& g) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  bool OnKeyDown(float x, float y, const IKeyPress& key) override;
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
  void OnMouseUp(float x, float y, const IMouseMod& mod) override;
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override;
  void OnEndAnimation() override;
  
  static int DeleteChars(ITextEntryControl* _this, size_t pos, size_t num);

  // Sole entry point for text into the buffer: stb_textedit routes both typed
  // input (STB_TEXTEDIT_INSERTCHARS) and paste (stb_textedit_paste) through here.
  static int InsertChars(ITextEntryControl* _this, size_t pos, const char16_t* text, size_t num);
  static void Layout(StbTexteditRow* row, ITextEntryControl* _this, int start_i);
  static float GetCharWidth(ITextEntryControl* _this, int n, int i);
  static char16_t GetChar(ITextEntryControl* _this, int pos);
  static int GetLength(ITextEntryControl* _this);

  bool EditInProgress() const { return mEditing; }
  void DismissEdit();
  void CommitEdit();

  /** Insert a UTF-8 string at the caret, replacing the selection. Used for text produced by the
   * platform's input method — dead-key composition, IME and Option/AltGr combinations never reach
   * a key-down handler as a plain character. Obeys SetMaxCodePoints(). */
  void InsertUTF8(const char* str);

  void SetPasswordMode(bool isPassword);
  void SetTabCommitCallback(std::function<void(IControl*)> cb) { mOnTabCommit = std::move(cb); }
  void SetMaxCodePoints(int max) { mMaxCodePoints = max; }

  // Reports the buffer on every change while editing, so a caller can react per keystroke instead
  // of waiting for the commit. Also fires once on a dismissed (cancelled) edit, with the string
  // the entry started from, so the last live value never outlives the edit that produced it.
  void SetChangeCallback(std::function<void(const char*)> cb) { mOnChange = std::move(cb); }

  /** Wrap the edited text over multiple rows at the control's width, instead of the single row
   * this control is otherwise limited to. Opt-in per edit session and reset on every
   * CreateTextEntry(), so single-line fields keep the original layout and drawing path.
   * @param wrap Whether to word-wrap
   * @param lineHeight Distance between row tops, in pixels */
  void SetWordWrap(bool wrap, float lineHeight) { mWordWrap = wrap; mLineHeight = lineHeight; }

  void CreateTextEntry(int paramIdx, const IText& text, const IRECT& bounds, int length, const char* str);

private:

  void SetStr(const char* str);
  void DrawWrapped(IGraphics& g);

  template<typename Proc>
  bool CallSTB(Proc proc);
  void OnStateChanged();
  void OnTextChange();
  void FillCharWidthCache();
  void CalcCursorSizes();
  float MeasureCharWidth(char16_t c, char16_t nc);
  void CopySelection();
  void Paste();
  void Cut();
  void SelectAll();
  
  bool mDrawCursor = false;
  bool mEditing = false;
  bool mIsPassword = false;
  int mMaxCodePoints = 0; // 0 = unlimited; reset on every CreateTextEntry
  bool mWordWrap = false; // reset on every CreateTextEntry
  float mLineHeight = 0.f;
  bool mRecursiveKeyGuard = false;
  bool mCursorIsSet = false;
  bool mCursorSizesValid = false;
  bool mNotifyTextChange = false;

  STB_TexteditState mEditState;
  WDL_TypedBuf<float> mCharWidths;
  std::u16string mEditString;
  std::string mInitialStr; // buffer at CreateTextEntry, replayed to mOnChange on a dismissed edit
  std::function<void(IControl*)> mOnTabCommit;
  std::function<void(const char*)> mOnChange;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
