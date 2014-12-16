/*
 * Copyright (C) 2013 Project Hatohol
 *
 * This file is part of Hatohol.
 *
 * Hatohol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3
 * as published by the Free Software Foundation.
 *
 * Hatohol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hatohol. If not, see <http://www.gnu.org/licenses/>.
 *
 * Additional permission under GNU GPL version 3 section 7
 *
 * If you modify this program, or any covered work, by linking or
 * combining it with the OpenSSL project's OpenSSL library (or a
 * modified version of that library), containing parts covered by the
 * terms of the OpenSSL or SSLeay licenses, Project Hatohol
 * grants you additional permission to convey the resulting work.
 * Corresponding Source for a non-source form of such a combination
 * shall include the source code for the parts of OpenSSL used as well
 * as that of the covered work.
 */

var HatoholHostgroupSelector = function(serverId, selectedCb) {
  var self = this;
  self.queryData = {"serverId": serverId};

  // call the constructor of the super class
  HatoholSelectorDialog.apply(
    this, ["hostgroup-selector", gettext("Hostgroup selecion"), selectedCb]);
  self.start("/hostgroup", "GET");
}

HatoholHostgroupSelector.prototype =
  Object.create(HatoholSelectorDialog.prototype);
HatoholHostgroupSelector.prototype.constructor = HatoholHostgroupSelector;

HatoholHostgroupSelector.prototype.makeQueryData = function() {
    return this.queryData;
}

HatoholHostgroupSelector.prototype.getNumberOfObjects = function(reply) {
  return reply.numberOfHosts;
}

HatoholHostgroupSelector.prototype.generateMainTable = function(tableId) {
  var html =
  '<table class="table table-condensed table-striped table-hover" id=' +
  tableId + '>' +
  '  <thead>' +
  '    <tr>' +
  '      <th>' + gettext("Server ID") + '</th>' +
  '      <th>' + gettext("Hostgroup ID") + '</th>' +
  '      <th>' + gettext("Hostgroup Name") + '</th>' +
  '    </tr>' +
  '  </thead>' +
  '  <tbody></tbody>' +
  '</table>'
  return html;
}

HatoholHostgroupSelector.prototype.generateTableRows = function(reply) {
  var s = "";
  this.setObjectArray(reply.hostgroups);
  for (var i = 0; i < reply.hostgroups.length; i++) {
    hostgroup = reply.hostgroups[i];
    s += '<tr>';
    s += '<td>' + hostgroup.serverId + '</td>';
    s += '<td>' + hostgroup.groupId + '</td>';
    s += '<td>' + hostgroup.groupName + '</td>';
    s += '</tr>';
  }
  return s;
}
