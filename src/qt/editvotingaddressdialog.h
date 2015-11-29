// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EDITVOTINGADDRESSDIALOG_H
#define EDITVOTINGADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
    class EditVotingAddressDialog;
}
class VotingTableModel;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

/** Dialog for editing a voting address and associated information.
 */
class EditVotingAddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditVotingAddressDialog(QWidget *parent = 0);
    ~EditVotingAddressDialog();

    void setModel(VotingTableModel *model);
    void loadRow(int row);

    QString getAddress() const;
    void setAddress(const QString &address);

public slots:
    void accept();

private:
    bool saveCurrentRow();

    Ui::EditVotingAddressDialog *ui;
    QDataWidgetMapper *mapper;
    VotingTableModel *model;

    QString address;
};

#endif // EDITVOTINGADDRESSDIALOG_H
