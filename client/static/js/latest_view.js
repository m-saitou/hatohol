/*
 * Copyright (C) 2013-2014 Project Hatohol
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

var LatestView = function(userProfile) {
  var self = this;
  var rawData, parsedData;

  self.reloadIntervalSeconds = 60;
  self.currentPage = 0;
  self.baseQuery = {
    limit: 50,
  };
  $.extend(self.baseQuery, getItemsQueryInURI());
  self.lastQuery = undefined;
  self.showToggleAutoRefreshButton();
  self.setupToggleAutoRefreshButtonHandler(load, self.reloadIntervalSeconds);

  // call the constructor of the super class
  HatoholMonitoringView.apply(this, [userProfile]);

  self.pager = new HatoholPager();
  self.userConfig = new HatoholUserConfig();
  start();

  function start() {
    var numRecordsPerPage;
    self.userConfig.get({
      itemNames:['num-items-per-page'],
      successCallback: function(conf) {
        self.baseQuery.limit =
          self.userConfig.findOrDefault(conf, 'num-items-per-page',
                                        self.baseQuery.limit);
        updatePager();
        setupFilterValues();
        setupCallbacks();
        load();
      },
      connectErrorCallback: function(XMLHttpRequest) {
        showXHRError(XMLHttpRequest);
      },
    });
  }

  function showXHRError(XMLHttpRequest) {
    var errorMsg = "Error: " + XMLHttpRequest.status + ": " +
      XMLHttpRequest.statusText;
    hatoholErrorMsgBox(errorMsg);
  }

  function saveConfig(items) {
    self.userConfig.store({
      items: items,
      successCallback: function() {
        // we just ignore it
      },
      connectErrorCallback: function(XMLHttpRequest) {
        showXHRError(XMLHttpRequest);
      },
    });
  }

  function updatePager() {
    self.pager.update({
      numTotalRecords: rawData ? rawData["totalNumberOfItems"] : -1,
      numRecordsPerPage: self.baseQuery.limit,
      selectPageCallback: function(page) {
        load(page);
        if (self.pager.numRecordsPerPage != self.baseQuery.limit) {
          self.baseQuery.limit = self.pager.numRecordsPerPage;
          saveConfig({'num-items-per-page': self.baseQuery.limit})
        }
      }
    });
  }

  function setupFilterValues(servers, query) {
    if (!servers && rawData && rawData.servers)
      servers = rawData.servers;

    if (!query)
      query = self.lastQuery ? self.lastQuery : self.baseQuery;

    self.setupHostFilters(servers, query);

    if ('limit' in query)
      $('#num-items-per-page').val(query.limit);
  }

  function setupCallbacks() {
    $("#table").stupidtable();
    $("#table").bind('aftertablesort', function(event, data) {
      var th = $(this).find("th");
      th.find("i.sort").remove();
      var icon = data.direction === "asc" ? "up" : "down";
      th.eq(data.column).append("<i class='sort glyphicon glyphicon-arrow-" + icon +"'></i>");
    });

    self.setupHostQuerySelectorCallback(
      load, '#select-server', '#select-host-group', '#select-host', '#select-application');
  }

  function parseData(replyData) {
    var parsedData = {};
    var appNames = [];
    var x, item;

    for (x = 0; x < replyData["applications"].length; ++x) {
      item = replyData["applications"][x];

      if (item["name"].length == 0)
        item["name"] = "_non_";
      else
        appNames.push(item["name"]);
    }
    parsedData.applications = appNames.uniq().sort();

    return parsedData;
  }

  function setLoading(loading) {
    if (loading) {
      $("#select-server").attr("disabled", "disabled");
      $("#select-host").attr("disabled", "disabled");
      $("#select-application").attr("disabled", "disabled");
    } else {
      $("#select-server").removeAttr("disabled");
      if ($("#select-host option").length > 1)
        $("#select-host").removeAttr("disabled");
    }
    $("#select-application").removeAttr("disabled");
  }

  function getGraphURL(item) {
    var url = "ajax_history?serverId=";
    url += escapeHTML(item["serverId"]);
    url += "&hostId=";
    url += escapeHTML(item["hostId"]);
    url += "&itemId=";
    url += escapeHTML(item["id"]);
    return url;
  }

  function getGraphLink(item) {
    if (!item || !item["valueType"] ||
        (item["valueType"] != hatohol.ITEM_INFO_VALUE_TYPE_FLOAT &&
         item["valueType"] != hatohol.ITEM_INFO_VALUE_TYPE_INTEGER)) {
      return "";
    }
    var link = "<a href='" + getGraphURL(item) + "'>"
    link += gettext("Graph");
    link += "</a>";
    return link;
  }

  function drawTableBody(replyData) {
    var serverName, hostName, clock, appName;
    var html = "", url, server, item, x;
    var targetAppName = self.getTargetAppName();

    html = "";
    for (x = 0; x < replyData["items"].length; ++x) {
      item       = replyData["items"][x];
      server     = replyData["servers"][item["serverId"]];
      url        = getItemGraphLocation(server, item["id"]);
      serverName = getServerName(server, item["serverId"]);
      hostName   = getHostName(server, item["hostId"]);
      clock      = item["lastValueTime"];
      appName    = item["itemGroupName"];

      if (targetAppName && appName != targetAppName)
        continue;

      html += "<tr><td>" + escapeHTML(serverName) + "</td>";
      html += "<td>" + escapeHTML(hostName) + "</td>";
      html += "<td>" + escapeHTML(appName) + "</td>";
      if (url)
        html += "<td><a href='" + url + "' target='_blank'>"
                + escapeHTML(item["brief"])  + "</a></td>";
      else
        html += "<td>" + escapeHTML(item["brief"])  + "</td>";
      html += "<td data-sort-value='" + escapeHTML(clock) + "'>" + formatDate(clock) + "</td>";
      html += "<td>" + formatItemLastValue(item) + "</td>";
      html += "<td>" + formatItemPrevValue(item) + "</td>";
      html += "<td>" + getGraphLink(item) + "</td>";
      html += "</tr>";
    }

    return html;
  }

  function drawTableContents(rawData) {
    $("#table tbody").empty();
    $("#table tbody").append(drawTableBody(rawData));
  }

  function updateCore(reply) {
    rawData = reply;
    parsedData = parseData(rawData);

    self.setServerFilterCandidates(rawData["servers"]);
    self.setHostgroupFilterCandidates(rawData["servers"]);
    self.setHostFilterCandidates(rawData["servers"]);
    self.setApplicationFilterCandidates(parsedData.applications);

    drawTableContents(rawData);
    self.pager.update({ numTotalRecords: rawData["totalNumberOfItems"] });
    setupFilterValues();
    setLoading(false);
    self.setAutoReload(load, self.reloadIntervalSeconds);
  }

  function getItemsQueryInURI() {
    var knownKeys = [
      "serverId", "hostgroupId", "hostId",
      "limit", "offset", "appName",
    ];
    var i, allParams = deparam(), query = {};
    for (i = 0; i < knownKeys.length; i++) {
      if (knownKeys[i] in allParams)
        query[knownKeys[i]] = allParams[knownKeys[i]];
    }
    return query;
  }

  function getQuery(page) {
    if (isNaN(page))
      page = 0;
    var query = $.extend({}, self.baseQuery, {
      limit:  self.baseQuery.limit,
      offset: self.baseQuery.limit * page
    });
    if (self.lastQuery) {
      $.extend(query, self.getHostFilterQuery());
      $.extend(query, query, { appName: self.getTargetAppName() });
    }
    self.lastQuery = query;
    return 'item?' + $.param(query);
  };

  function load(page) {
    self.displayUpdateTime();
    setLoading(true);
    if (!isNaN(page)) {
      self.currentPage = page;
    }
    self.startConnection(getQuery(self.currentPage), updateCore);
    self.pager.update({ currentPage: self.currentPage });
  }
};

LatestView.prototype = Object.create(HatoholMonitoringView.prototype);
LatestView.prototype.constructor = LatestView;
