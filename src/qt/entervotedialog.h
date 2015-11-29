// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ENTERVOTEDIALOG_H
#define ENTERVOTEDIALOG_H

#include <QDialog>

namespace Ui {
    class EntervoteDialog;
}
class WalletModel;

/** Dialog for entering how much to vote for a particular nation.
 */
class EntervoteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EntervoteDialog(std::string& nation, QWidget *parent = 0);
    ~EntervoteDialog();

    void setModel(WalletModel *model);

    QString getAddress() const;
    void setAddress(const QString &address);

    QString getNation() const;
    void setNation(const QString &nation);

    QString getAmount() const;
    void setAmount(const QString &amount);

public slots:
    void accept();

private:
    bool submitVote();

    Ui::EntervoteDialog *ui;
    WalletModel *model;

    QString address;
    QString nation;
    QString amount;
};

#endif // ENTERVOTEDIALOG_H
