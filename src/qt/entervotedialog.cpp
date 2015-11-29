// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "entervotedialog.h"
#include "ui_entervotedialog.h"

#include "addresstablemodel.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>

EntervoteDialog::EntervoteDialog(std::string& nation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EntervoteDialog), model(0)
{
    ui->setupUi(this);

    GUIUtil::setupAddressWidget(ui->addressEdit, this);

    setWindowTitle(tr("Enter Vote Amount for Empire"));

    std::string voteStr = "Vote for " + nation;
    ui->labelEdit->setText(QString::fromStdString(voteStr));

    ui->labelEdit->setEnabled(false);
    ui->addressEdit->setEnabled(false);
    ui->voteAmountEdit->setEnabled(true);
}

EntervoteDialog::~EntervoteDialog()
{
    delete ui;
}

void EntervoteDialog::setModel(WalletModel *model)
{
    this->model = model;
}

bool EntervoteDialog::submitVote()
{
    if(!model)
        return false;

    nation = ui->labelEdit->text();
    address = ui->addressEdit->text();
    amount = ui->voteAmountEdit->text();

    return !address.isEmpty() && !amount.isEmpty();
}

void EntervoteDialog::accept()
{
    if (!model)
        return;

    if (submitVote())
        QDialog::accept();
}

QString EntervoteDialog::getAddress() const
{
    return address;
}

void EntervoteDialog::setAddress(const QString &address)
{
    this->address = address;
    ui->addressEdit->setText(address);
}

QString EntervoteDialog::getNation() const
{
    return nation;
}

void EntervoteDialog::setNation(const QString &nation)
{
    this->nation = nation;
    ui->labelEdit->setText(nation);
}

QString EntervoteDialog::getAmount() const
{
    return amount;
}

void EntervoteDialog::setAmount(const QString &amount)
{
    this->amount = amount;
    ui->voteAmountEdit->setText(amount);
}
