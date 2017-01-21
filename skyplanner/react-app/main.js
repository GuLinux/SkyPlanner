import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer, NotificationManager } from 'react-notifications';
import Ajax from './ajax';
import AuthManager from './auth-manager';
import URLs from './urls';
require('style!react-notifications/lib/notifications.css');

var history = hashHistory;

var requireAuth = function() {
    if (!AuthManager.user() ) {
        replace({
            pathname: '/login',
            state: { nextPathname: nextState.location.pathname }
        })
    }
}


render(
    <div>
        <Router history={history} ref='router'>
            <Route path={URLs.root.route} component={SkyPlannerApp}>
                <IndexRoute component={SkyPlannerHomePage} />} />
                <Route path={URLs.login.route} component={SkyPlannerLoginPage} />
                <Route path={URLs.logout.route} component='div' onEnter={() => AuthManager.setUser(null) } />
            </Route>
        </Router>

        <NotificationContainer />
    </div>,
    document.getElementById('content')
);


