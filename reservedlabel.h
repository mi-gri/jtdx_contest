// -*- Mode: C++ -*-
/* CE3TSK 2026-10-26 (TODO.md 3.-211, JA1NLX's report): the Pwr column - the power label, the Tx level slider, the SWR
   label - changed size at every transmission. With Settings' Output power the SWR label was empty while receiving and
   read "swr1.30" while transmitting, the widest text of the column, so the column, and what its row gives the controls
   beside it, grew: ~16 px in the alternative layout (the Tx buttons moved), ~14 px in the standard one (the clock and the
   buttons under it). And the power label went from " Pwr" to two lines at the first reading, shortening the slider.

   A ReservedLabel is never hinted smaller than the largest of some texts it may show - its size hint and its minimum
   both, so a row short of room squeezes it no further while empty than while it shows its widest text (measured: an
   empty label's minimum is nothing, and the row gave the slider the difference). Each text is measured on a hidden
   twin, a child QLabel with the label's font, style sheets, alignment and format, so rich text is measured as QLabel
   measures it. No reserve, a QLabel as before. MainWindow reserves the two labels while Output power is on (a station
   without it keeps the narrower column); the minimum it reserves is what a transmission needed already.

   The SWR reading is two lines, "swr" over the value with two decimals always, centred - narrower than the one line was
   (the operator). The reserves: the readings below 100 and the powers below 1000 W made of one digit each ("8.88",
   "88.88", "888 W" - widths add up, so no mix of digits is wider). A reading of 100 or 1000 W and more would still
   widen the column. No Q_OBJECT: nothing for moc. */

#ifndef RESERVEDLABEL_H
#define RESERVEDLABEL_H

#include <QEvent>
#include <QLabel>
#include <QPair>
#include <QSize>
#include <QString>
#include <QStringList>

class ReservedLabel : public QLabel
{
public:
  using QLabel::QLabel;

  // the texts whose size the label keeps; none, a QLabel as before
  void setReserve (QStringList const& texts)
  {
    if (texts == reserve_) return;
    reserve_ = texts;
    if (!twin_ && !reserve_.isEmpty ())
      {
        twin_ = new QLabel {this};   // a child: the label's font and style sheets reach it as they reach the label
        twin_->hide ();
      }
    valid_ = false;
    updateGeometry ();
  }
  QStringList reserve () const { return reserve_; }

  QSize sizeHint () const override
  {
    QSize const hint {QLabel::sizeHint ()};
    return reserve_.isEmpty () ? hint : hint.expandedTo (measured ().first);
  }
  QSize minimumSizeHint () const override
  {
    QSize const hint {QLabel::minimumSizeHint ()};
    return reserve_.isEmpty () ? hint : hint.expandedTo (measured ().second);
  }

protected:
  void changeEvent (QEvent * e) override
  {
    if (e->type () == QEvent::FontChange || e->type () == QEvent::StyleChange) valid_ = false;
    QLabel::changeEvent (e);
  }

private:
  // the largest size hint and minimum of the reserved texts, on the twin set up as the label is
  QPair<QSize, QSize> measured () const
  {
    if (valid_) return measured_;
    twin_->setAlignment (alignment ());
    twin_->setTextFormat (textFormat ());
    twin_->setMargin (margin ());
    twin_->setIndent (indent ());
    twin_->setWordWrap (wordWrap ());
    twin_->ensurePolished ();
    QSize hint, minimum;
    for (auto const& t : reserve_)
      {
        twin_->setText (t);
        hint = hint.expandedTo (twin_->sizeHint ());
        minimum = minimum.expandedTo (twin_->minimumSizeHint ());
      }
    measured_ = {hint, minimum};
    valid_ = true;
    return measured_;
  }

  QStringList reserve_;
  QLabel * twin_ {nullptr};
  mutable QPair<QSize, QSize> measured_;
  mutable bool valid_ {false};
};

// the Pwr column's texts
namespace pwr_column
{
  // the SWR reading as the label shows it, from hundredths (1.30 is 130): "swr" over the value, two decimals always
  inline QString swrText (unsigned swr)
  {
    return QString {"swr\n%1"}.arg (swr / 100., 0, 'f', 2);
  }

  // the readings below 100 made of one digit each, in both forms
  inline QStringList swrWidest ()
  {
    QStringList texts;
    for (QChar d = '0'; d <= '9'; d = QChar (d.unicode () + 1))
      texts << QString {"swr\n%1.%1%1"}.arg (d) << QString {"swr\n%1%1.%1%1"}.arg (d);
    return texts;
  }

  // the power label's format (translated, %1 the watts) with the powers below 1000 W made of one digit each
  inline QStringList wattsWidest (QString const& format)
  {
    QStringList texts;
    for (QChar d = '0'; d <= '9'; d = QChar (d.unicode () + 1)) texts << format.arg (QString {3, d});
    return texts;
  }
}

#endif
