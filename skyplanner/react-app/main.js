import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import SkyPlannerRegistrationPage from './skyplanner-registrationpage'
import SkyPlannerGearPage from './skyplanner-gearpage'
import SkyPlannerObservationsPage from './skyplanner-observationspage'
import { NotificationContainer, NotificationManager } from 'react-notifications';
import AuthManager from './auth-manager';
import URLs from './urls';
require('style!react-notifications/lib/notifications.css');
import { api } from './skyplanner-api'

var history = hashHistory;

var requireAuth = function(nextState, replace) {
    if (!AuthManager.user() ) {
        replace({
            pathname: URLs.login.path,
            state: { nextPathname: nextState.location.pathname }
        })
    }
}

var requireAnonymous = function(nextState, replace) {
    if(!!AuthManager.user()) {
        replace(URLs.root.path);
    }
}


var renderRoot = function() {
    render(
        <div>
            <Router history={history}>
                <Route path={URLs.root.route} component={SkyPlannerApp}>
                    <IndexRoute component={SkyPlannerHomePage} />} />
                    <Route path={URLs.login.route} component={SkyPlannerLoginPage} onEnter={requireAnonymous}/>
                    <Route path={URLs.register.route} component={SkyPlannerRegistrationPage} onEnter={requireAnonymous}/>
                    <Route path={URLs.gear.route} component={SkyPlannerGearPage} onEnter={requireAuth} />
                    <Route path={URLs.observations.route} component={SkyPlannerObservationsPage} onEnter={requireAuth} />
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

    api.tokenLogin(token, () => renderRoot(), () => renderRoot() );
} else {
    renderRoot()
}
