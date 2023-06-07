#ifndef DELEGATES_H
#define DELEGATES_H

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTextBrowser>
#include <QTextEdit>
#include "tableWidget_global.h"

class TABLE_EXPORT DateTimeDelegate : public QStyledItemDelegate {
   public:
    DateTimeDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QDateTimeEdit* editor = new QDateTimeEdit(parent);
        editor->setMinimumWidth(200);

        editor->setDisplayFormat("yyyy-MM-dd hh:mm:ss AP");
        editor->setCalendarPopup(true);

        QString dateTimeStr = index.data().toString();
        if (!dateTimeStr.isEmpty()) {
            QDateTime dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
            editor->setDateTime(dateTime);
        } else {
            editor->clear();
        }

        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QDateTimeEdit* dateTimeEditor = static_cast<QDateTimeEdit*>(editor);
        QString dateTimeStr = index.data().toString();
        if (!dateTimeStr.isEmpty()) {
            QDateTime dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
            dateTimeEditor->setDateTime(dateTime);
        } else {
            dateTimeEditor->clear();
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QDateTimeEdit* dateTimeEditor = static_cast<QDateTimeEdit*>(editor);
        QString dateTimeStr = dateTimeEditor->dateTime().toString(Qt::ISODate);
        model->setData(index, dateTimeStr);
    }
};

class TABLE_EXPORT DateDelegate : public QStyledItemDelegate {
   public:
    DateDelegate(QObject* parent = nullptr, QDate defaultDate = QDate::currentDate(),
                 QDate minDate = QDate(), QDate maxDate = QDate())
        : QStyledItemDelegate(parent),
          defaultDate(defaultDate),
          minDate(minDate),
          maxDate(maxDate) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QDateEdit* editor = new QDateEdit(parent);

        // If minDate is set
        if (minDate != QDate()) {
            editor->setMinimumDate(minDate);
        }

        // If maxDate is set
        if (maxDate != QDate()) {
            editor->setMaximumDate(maxDate);
        }

        editor->setDate(defaultDate);
        editor->setMinimumWidth(120);

        editor->setDisplayFormat("yyyy-MM-dd");
        editor->setCalendarPopup(true);
        editor->setDateTime(QDateTime::fromString(index.data().toString(), "yyyy-MM-dd"));
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QDateTimeEdit* dateEditor = static_cast<QDateTimeEdit*>(editor);
        dateEditor->setDateTime(QDateTime::fromString(index.data().toString(), "yyyy-MM-dd"));
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QDateTimeEdit* dateEditor = static_cast<QDateTimeEdit*>(editor);
        QString dateStr = dateEditor->dateTime().toString("yyyy-MM-dd");
        model->setData(index, dateStr);
    }

   private:
    QDate minDate, maxDate, defaultDate;
};

class TABLE_EXPORT TimeDelegate : public QStyledItemDelegate {
   public:
    TimeDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QTimeEdit* editor = new QTimeEdit(parent);
        editor->setMinimumWidth(120);
        editor->setDisplayFormat("hh:mm:ss AP");

        QString timeStr = index.data().toString();
        if (!timeStr.isEmpty() && timeStr != "null") {
            QTime time = QTime::fromString(timeStr, Qt::ISODate);
            editor->setTime(time);
        } else {
            editor->clear();
        }
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QTimeEdit* timeEditor = static_cast<QTimeEdit*>(editor);
        QString timeStr = index.data().toString();
        if (!timeStr.isEmpty() && timeStr != "null") {
            QTime time = QTime::fromString(timeStr, Qt::ISODate);
            timeEditor->setTime(time);
        } else {
            timeEditor->clear();
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QTimeEdit* timeEditor = static_cast<QTimeEdit*>(editor);
        QString timeStr = timeEditor->time().toString(Qt::ISODate);
        model->setData(index, timeStr);
    }
};

class TABLE_EXPORT SpinBoxDelegate : public QStyledItemDelegate {
   public:
    SpinBoxDelegate(QObject* parent = nullptr, int min = 0, int max = 100)
        : QStyledItemDelegate(parent), min(min), max(max) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QSpinBox* editor = new QSpinBox(parent);

        editor->setMinimum(min);
        editor->setMaximum(max);
        editor->setValue(index.data().toInt());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(index.data().toInt());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        model->setData(index, spinBox->value());
    }

   private:
    int min, max;
};

class TABLE_EXPORT TextEditDelegate : public QStyledItemDelegate {
   public:
    TextEditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QTextEdit* editor = new QTextEdit(parent);

        editor->setPlainText(index.data().toString());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QTextEdit* textEdit = static_cast<QTextEdit*>(editor);
        textEdit->setPlainText(index.data().toString());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QTextEdit* textEdit = static_cast<QTextEdit*>(editor);
        model->setData(index, textEdit->toPlainText());
    }
};

class TABLE_EXPORT TextBrowserDelegate : public QStyledItemDelegate {
   public:
    TextBrowserDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QTextBrowser* editor = new QTextBrowser(parent);

        editor->setHtml(index.data().toString());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QTextBrowser* textBrowser = static_cast<QTextBrowser*>(editor);
        textBrowser->setPlainText(index.data().toString());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QTextBrowser* textBrowser = static_cast<QTextBrowser*>(editor);
        model->setData(index, textBrowser->toHtml());
    }
};

class TABLE_EXPORT LineEditDelegate : public QStyledItemDelegate {
   public:
    LineEditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QLineEdit* editor = new QLineEdit(parent);

        editor->setText(index.data().toString());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(index.data().toString());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text());
    }
};

class TABLE_EXPORT ComboBoxDelegate : public QStyledItemDelegate {
   public:
    ComboBoxDelegate(QObject* parent = nullptr, const QStringList& items = QStringList())
        : QStyledItemDelegate(parent), items(items) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QComboBox* editor = new QComboBox(parent);

        editor->addItems(items);
        editor->setCurrentText(index.data().toString());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(index.data().toString());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentText());
    }

   private:
    QStringList items;
};

class TABLE_EXPORT RadioButtonDelegate : public QStyledItemDelegate {
   public:
    RadioButtonDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QRadioButton* editor = new QRadioButton(parent);

        editor->setChecked(index.data().toBool());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QRadioButton* radioButton = static_cast<QRadioButton*>(editor);
        radioButton->setChecked(index.data().toBool());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) {
        QRadioButton* radioButton = static_cast<QRadioButton*>(editor);
        model->setData(index, radioButton->isChecked());
    }
};

class TABLE_EXPORT CheckBoxDelegate : public QStyledItemDelegate {
   public:
    CheckBoxDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QCheckBox* editor = new QCheckBox(parent);

        editor->setChecked(index.data().toBool());
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
        checkBox->setChecked(index.data().toBool());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
        model->setData(index, checkBox->isChecked());
    }
};

class TABLE_EXPORT DoubleSpinBoxDelegate : public QStyledItemDelegate {
   public:
    DoubleSpinBoxDelegate(QObject* parent = nullptr, int decimals = 2, int min = 0, int max = 100)
        : QStyledItemDelegate(parent), decimals(decimals), min(min), max(max) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override {
        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);

        editor->setValue(index.data().toDouble());
        editor->setDecimals(decimals);
        editor->setMaximum(max);
        editor->setMinimum(min);
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(index.data().toDouble());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
        model->setData(index, spinBox->value());
    }

   private:
    int decimals, max, min;
};

#endif  // DELEGATES_H
