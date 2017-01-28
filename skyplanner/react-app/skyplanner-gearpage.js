import React from 'react'
import { Table } from 'react-bootstrap'

class TelescopeRow extends React.Component {
    render() {
        return (
            <tr>
                <td>{this.props.telescope.name}</td>
                <td>{this.props.telescope.focal_length}</td>
                <td>{this.props.telescope.diameter}</td>
            </tr>
        );
    }
}

class TelescopesFrame extends React.Component {
    render() {
        return (
            <div>
                <h3>Telescopes</h3>
                <Table bordered condensed hover>
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Focal Length</th>
                            <th>Diameter</th>
                        </tr>
                    </thead>
                    <tbody>
                        {this.rows()}
                    </tbody>
                </Table>
            </div>
        );
    }

    rows() {
        return this.props.telescopes.map( (t) => <TelescopeRow telescope={t} /> );
    }
}

class SkyPlannerGearPage extends React.Component {
    render() {
        return (
            <div className='container'>
                <TelescopesFrame telescopes={ [{id: 1, name: "Dob", focal_length: 1200, diameter: 300}] }/>
            </div>
        );
    }
}

export default SkyPlannerGearPage;
