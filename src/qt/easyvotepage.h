// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EASYVOTEPAGE_H
#define EASYVOTEPAGE_H

#include <QWidget>

namespace Ui {
    class EasyvotePage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Easy voting page widget */
class EasyvotePage : public QWidget
{
    Q_OBJECT

public:
    explicit EasyvotePage(QWidget *parent = 0);
    ~EasyvotePage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

public slots:
    void showVoteForBrazil();
    void showVoteForChina();
    void showVoteForFrance();
    void showVoteForGermany();
    void showVoteForIndia();
    void showVoteForIndonesia();
    void showVoteForIran();
    void showVoteForItaly();
    void showVoteForJapan();
    void showVoteForMexico();
    void showVoteForNigeria();
    void showVoteForPakistan();
    void showVoteForRussia();
    void showVoteForTurkey();
    void showVoteForUnitedKingdom();
    void showVoteForUnitedStates();

private:
    void showVoteForNation(std::string nation);

    Ui::EasyvotePage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
};

#endif // EASYVOTEPAGE_H
