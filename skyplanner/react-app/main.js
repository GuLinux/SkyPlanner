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

var requireAuth = function(nextState, replace) {
    if (!AuthManager.user() ) {
        replace({
            pathname: URLs.login.path,
            state: { nextPathname: nextState.location.pathname }
        })
    }
}

var requireAnonymout = function(nextState, replace) {
    if(AuthManager.user()) {
        replace(URLs.root.path);
    }
}


var renderRoot = function() {
    render(
        <div>
            <Router history={history}>
                <Route path={URLs.root.route} component={SkyPlannerApp}>
                    <IndexRoute component={SkyPlannerHomePage} />} />
                    <Route path={URLs.login.route} component={SkyPlannerLoginPage} onEnter={requireAnonymout}/>
                    <Route path={URLs.equipment.route} component={ (props) => <div>Equipment</div> } onEnter={requireAuth} />
                    <Route path={URLs.logout.route} component='div' onEnter={() => AuthManager.logout() } />
                </Route>
            </Router>

            <NotificationContainer />
        </div>,
        document.getElementById('content')
    );
}


let token = AuthManager.token();
if(token) {
    Ajax.fetch( URLs.buildAuthPath('/api/users/get'))
        .then(Ajax.decode_json({
            is_success: (r) => r.status == 200, 
            success: (j) => {
                Object.assign(j, {token: token});
                AuthManager.login(j);
                renderRoot();
            },
            failure: (j, r) => {
                renderRoot();
            } 
    }));
} else {
    renderRoot()
}
