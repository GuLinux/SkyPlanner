import React from 'react'
import { Table, Button, ButtonGroup, FormControl, FormGroup } from 'react-bootstrap'
import URLs from './urls'
import { NotificationManager } from 'react-notifications';
import { api, Status } from './skyplanner-api'

class AddNewObservation extends React.Component {
    render() {
        return <p>new observations form</p>;
    }
}

class ObservationsTable extends React.Component {
    render() {
        return <p>Observations table</p>;
    }
}

class SkyPlannerObservationsPage extends React.Component {
    render() {
        return (
            <div className='container'>
                <AddNewObservation />
                <ObservationsTable />
            </div>
        );
    }
}

export default SkyPlannerObservationsPage;
