import React from 'react'
import ReactDOM from 'react-dom'

import SkyPlannerNavigation from './skyplanner-navigation';
import SkyPlannerContentManager from './skyplanner-content-manager';
import SkyPlannerHomepage from './skyplanner-homepage';

ReactDOM.render(
    <SkyPlannerNavigation />,
    document.getElementById('nav')
);
/*
ReactDOM.render(
    <SkyPlannerContentManager />,
    document.getElementById('content')
);
*/
