// Copyright (c) 2011-2013 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "editvotingaddressdialog.h"
#include "ui_editvotingaddressdialog.h"

#include "votingtablemodel.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>

EditVotingAddressDialog::EditVotingAddressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditVotingAddressDialog), mapper(0), model(0)
{
    ui->setupUi(this);

    GUIUtil::setupAddressWidget(ui->addressEdit, this);
    GUIUtil::setupAddressWidget(ui->nationEdit, this);

    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

EditVotingAddressDialog::~EditVotingAddressDialog()
{
    delete ui;
}

void EditVotingAddressDialog::setModel(VotingTableModel *model)
{
    this->model = model;
    if(!model)
        return;

    mapper->setModel(model);
    mapper->addMapping(ui->labelEdit, VotingTableModel::Label);
    mapper->addMapping(ui->addressEdit, VotingTableModel::Address);
    mapper->addMapping(ui->nationEdit, VotingTableModel::Nation);
}

void EditVotingAddressDialog::loadRow(int row)
{
    mapper->setCurrentIndex(row);
}

bool EditVotingAddressDialog::saveCurrentRow()
{
    if(!model)
        return false;

    address = model->addRow(
        ui->labelEdit->text(),
        ui->addressEdit->text(),
        ui->nationEdit->text());
    return !address.isEmpty();
}

void EditVotingAddressDialog::accept()
{
    if(!model)
        return;

    if(!saveCurrentRow())
    {
        switch(model->getEditStatus())
        {
        case VotingTableModel::OK:
            // Failed with unknown reason. Just reject.
            break;
        case VotingTableModel::NO_CHANGES:
            // No changes were made during edit operation. Just reject.
            break;
        case VotingTableModel::INVALID_ADDRESS:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered address \"%1\" is not a valid EmpireCoin address.").arg(ui->addressEdit->text()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case VotingTableModel::DUPLICATE_ADDRESS:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered address \"%1\" is already in the address book.").arg(ui->addressEdit->text()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case VotingTableModel::WALLET_UNLOCK_FAILURE:
            QMessageBox::critical(this, windowTitle(),
                tr("Could not unlock wallet."),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case VotingTableModel::KEY_GENERATION_FAILURE:
            QMessageBox::critical(this, windowTitle(),
                tr("New key generation failed."),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        }
        return;
    }
    QDialog::accept();
}

QString EditVotingAddressDialog::getAddress() const
{
    return address;
}

void EditVotingAddressDialog::setAddress(const QString &address)
{
    this->address = address;
    ui->addressEdit->setText(address);
}
