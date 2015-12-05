// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "easyvotepage.h"
#include "ui_easyvotepage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "empirecoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "entervotedialog.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#include "easyvotepage.moc"

EasyvotePage::EasyvotePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EasyvotePage),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    // connect a signal to all empire nation buttons
    connect(ui->pushButton_China, SIGNAL(clicked()), this,
            SLOT(showVoteForChina()));
    connect(ui->pushButton_USA, SIGNAL(clicked()), this,
            SLOT(showVoteForUSA()));
    connect(ui->pushButton_India, SIGNAL(clicked()), this,
            SLOT(showVoteForIndia()));
    connect(ui->pushButton_Brazil, SIGNAL(clicked()), this,
            SLOT(showVoteForBrazil()));
    connect(ui->pushButton_Indonesia, SIGNAL(clicked()), this,
            SLOT(showVoteForIndonesia()));
    connect(ui->pushButton_Japan, SIGNAL(clicked()), this,
            SLOT(showVoteForJapan()));
    connect(ui->pushButton_Russia, SIGNAL(clicked()), this,
            SLOT(showVoteForRussia()));
    connect(ui->pushButton_Germany, SIGNAL(clicked()), this,
            SLOT(showVoteForGermany()));
    connect(ui->pushButton_Mexico, SIGNAL(clicked()), this,
            SLOT(showVoteForMexico()));
    connect(ui->pushButton_Nigeria, SIGNAL(clicked()), this,
            SLOT(showVoteForNigeria()));
    connect(ui->pushButton_France, SIGNAL(clicked()), this,
            SLOT(showVoteForFrance()));
    connect(ui->pushButton_UK, SIGNAL(clicked()), this,
            SLOT(showVoteForUK()));
    connect(ui->pushButton_Pakistan, SIGNAL(clicked()), this,
            SLOT(showVoteForPakistan()));
    connect(ui->pushButton_Italy, SIGNAL(clicked()), this,
            SLOT(showVoteForItaly()));
    connect(ui->pushButton_Turkey, SIGNAL(clicked()), this,
            SLOT(showVoteForTurkey()));
    connect(ui->pushButton_Iran, SIGNAL(clicked()), this,
            SLOT(showVoteForIran()));
}

EasyvotePage::~EasyvotePage()
{
    delete ui;
}

void EasyvotePage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
}

void EasyvotePage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void EasyvotePage::showVoteForChina()
{
    showVoteForNation("China");
}

void EasyvotePage::showVoteForUSA()
{
    showVoteForNation("USA");
}

void EasyvotePage::showVoteForIndia()
{
    showVoteForNation("India");
}

void EasyvotePage::showVoteForBrazil()
{
    showVoteForNation("Brazil");
}

void EasyvotePage::showVoteForIndonesia()
{
    showVoteForNation("Indonesia");
}

void EasyvotePage::showVoteForJapan()
{
    showVoteForNation("Japan");
}

void EasyvotePage::showVoteForRussia()
{
    showVoteForNation("Russia");
}

void EasyvotePage::showVoteForGermany()
{
    showVoteForNation("Germany");
}

void EasyvotePage::showVoteForMexico()
{
    showVoteForNation("Mexico");
}

void EasyvotePage::showVoteForNigeria()
{
    showVoteForNation("Nigeria");
}

void EasyvotePage::showVoteForFrance()
{
    showVoteForNation("France");
}

void EasyvotePage::showVoteForUK()
{
    showVoteForNation("UK");
}

void EasyvotePage::showVoteForPakistan()
{
    showVoteForNation("Pakistan");
}

void EasyvotePage::showVoteForItaly()
{
    showVoteForNation("Italy");
}

void EasyvotePage::showVoteForTurkey()
{
    showVoteForNation("Turkey");
}

void EasyvotePage::showVoteForIran()
{
    showVoteForNation("Iran");
}

void EasyvotePage::showVoteForNation(std::string nation)
{
    if(this->walletModel)
    {
        int nationIndex =
            this->walletModel->getEmpireCoinNationIndex(nation);
        std::string nationStr =
            this->walletModel->getEmpireCoinVotingTxDestination(nationIndex);

        EntervoteDialog dlg(nation, this);
        dlg.setModel(this->walletModel);
        dlg.setAddress(QString::fromStdString(nationStr));
        dlg.setAmount(QString(""));
        if (dlg.exec() == QDialog::Accepted)
        {
            QString vote_amount = dlg.getAmount();

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                this, "Vote Confirmation",
                "You are about to send " + vote_amount +
                " EMP as a vote for " + QString::fromStdString(nation) +
                ".  Are you sure you'd like to cast this vote?",
                QMessageBox::Yes|QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                SendCoinsRecipient vote;
                vote.address = dlg.getAddress();
                // if we ever see this label, it's because it was a
                // winning round that we've received back on payout.
                vote.label = QString("Submitted vote for ") + vote.address;
                vote.amount = (vote_amount.toULongLong() * 100000000);

                printf("Submitting a vote for %s in the amount of %lld\n",
                       vote.address.toStdString().c_str(), (vote.amount / 100000000));

                QList<SendCoinsRecipient> l;
                l.append(vote);

                WalletModel::UnlockContext ctx(walletModel->requestUnlock());
                if (ctx.isValid())
                {
                    QMessageBox msgBox;
                    const WalletModel::SendCoinsReturn ret = walletModel->sendCoins(l, NULL);
                    switch(ret.status)
                    {
                        case WalletModel::InvalidAddress:
                            msgBox.setText("You've specified an invalid voting amount.");
                            break;
                        case WalletModel::InvalidAmount:
                            msgBox.setText("You've specified an invalid voting amount.");
                            break;
                        case WalletModel::DuplicateAddress:
                            msgBox.setText("Duplicate Address.");
                            break;
                        case WalletModel::AmountExceedsBalance:
                            msgBox.setText("The voting amount exceeds your current balance.");
                            break;
                        case WalletModel::AmountWithFeeExceedsBalance:
                        {
                            char buf[16] = {0};
                            snprintf(buf, 16, "%lld", (ret.fee / 100000000));
                            QString tmp = "The voting amount with the transaction fee of " +
                                QString(buf) + " exceeds your current balance.";
                            msgBox.setText(tmp);
                            break;
                        }
                        case WalletModel::TransactionCreationFailed:
                            msgBox.setText("The transaction failed to be created.");
                            break;
                        case WalletModel::TransactionCommitFailed:
                            msgBox.setText("The transaction failed to be committed.");
                            break;
                        case WalletModel::OK:
                        {
                            // This was originally for debugging
                            /* QString tmp = "Transaction confirmed with tx hash " + ret.hex; */
                            /* msgBox.setText(tmp); */
                            return;
                        }
                        case WalletModel::Aborted:
                            QString tmp = "Transaction aborted.";
                            msgBox.setText(tmp);
                            break;
                    }
                    msgBox.exec();
                }
            }
        }
    }
}
