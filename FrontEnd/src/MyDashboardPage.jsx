// File: MyDashboardPage.jsx

// Importing necessary libraries and styles
import React, { useState, useEffect, useRef } from 'react';
import L from 'leaflet';
import 'leaflet/dist/leaflet.css';
import { Chart, CategoryScale, LinearScale, LineController, LineElement, PointElement, Tooltip, Legend } from 'chart.js';
import './MyDashboardPage.css'; // Import the CSS file

Chart.register(CategoryScale, LinearScale, LineController, LineElement, PointElement, Tooltip, Legend);

// MyDashboardPage component definition
const MyDashboardPage = () => {
  // State hooks for managing selected data, options, JSON data, map instance, and map initialization
  const [selectedOption, setSelectedOption] = useState('');
  const [selectedData, setSelectedData] = useState(null);
  const [options, setOptions] = useState([]);
  const [jsonData, setJsonData] = useState([]);
  const [map, setMap] = useState(null); // State to hold the map instance
  const mapInitialized = useRef(false);

  // Fetch data from the server on component mount
  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch('http://13.60.21.191:3000/api/');
        const jsonData = await response.json();

        if (Array.isArray(jsonData) && jsonData.length > 0 && jsonData[0].date) {
          const dataOptions = jsonData.map(item => item.date) || [];

          setOptions(dataOptions);
          setJsonData(jsonData);

          if (dataOptions.length > 0) {
            setSelectedOption(dataOptions[0]);
            setSelectedData(jsonData[0]);
          }

          // Create the map when data is fetched
          createMap(jsonData[0]);
          updateAltitudesGraph(jsonData[0]);
        } else {
          console.error('Invalid JSON structure:', jsonData);
        }
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();
  }, []); // Empty dependency array ensures the effect runs only once on mount

  // Handle dropdown change event
  const handleDropdownChange = (event) => {
    const selectedDataValue = event.target.value;
    setSelectedOption(selectedDataValue);

    const selectedDataItem = jsonData.find(item => item.date === selectedDataValue);
    setSelectedData(selectedDataItem);

    // Update the map and altitudes graph when the dropdown changes
    updateMap(selectedDataItem);
    updateAltitudesGraph(selectedDataItem);
  };

  // Create the Leaflet map
  const createMap = (firstRun) => {
    if (!mapInitialized.current) {
      const mymap = L.map('map').setView([parseFloat(firstRun.positions[0].lat), parseFloat(firstRun.positions[0].lng)], 15);

      L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: 'Rigraziamento a Faddi e Panci'
      }).addTo(mymap);

      const coordinates = firstRun.positions.map(position => [parseFloat(position.lat), parseFloat(position.lng)]);
      const polyline = L.polyline(coordinates, { color: 'blue' }).addTo(mymap);

      mymap.fitBounds(polyline.getBounds());

      setMap(mymap); // Save the map instance in state
      mapInitialized.current = true; // Set the flag to true
    }
  };

  // Update the Leaflet map
  const updateMap = (selectedDataItem) => {
    if (map && selectedDataItem) {
      const coordinates = selectedDataItem.positions.map(position => [parseFloat(position.lat), parseFloat(position.lng)]);
      const polyline = L.polyline(coordinates, { color: 'blue' });

      // Clear previous layers and add the new polyline
      map.eachLayer(layer => {
        if (layer instanceof L.Polyline) {
          layer.remove();
        }
      });

      polyline.addTo(map);
      map.fitBounds(polyline.getBounds());
    }
  };

  // Update the altitudes graph
  const updateAltitudesGraph = (selectedDataItem) => {
    if (selectedDataItem) {
      const positions = selectedDataItem.positions;
      const altitudes = positions.map(position => position.alt);

      // Calculate time in mm:ss format for each position
      const timeInMinutes = positions.map((_, index) => {
        const interval = selectedDataItem.duration / altitudes.length; // Set the interval in seconds
        const currentTime = (index * interval);
        const minutes = Math.floor(currentTime / 60);
        const seconds = Math.floor(currentTime % 60);
        return `${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
      });

      const ctx = document.getElementById('altitudesChart');
      if (ctx) {
        // Destroy the existing chart instance if it exists
        Chart.getChart(ctx)?.destroy();

        // Create a new altitudes chart
        const altitudesChart = new Chart(ctx, {
          type: 'line',
          data: {
            labels: timeInMinutes, // Use time in mm:ss format as labels
            datasets: [{
              label: 'Altitudes',
              data: altitudes,
              borderColor: 'rgba(75, 192, 192, 1)',
              borderWidth: 1,
              fill: false,
            }],
          },
          options: {
            scales: {
              x: {
                type: 'category', // Use category scale for mm:ss format
                labels: timeInMinutes,
                position: 'bottom',
                title: {
                  display: true,
                  text: 'Time (mm:ss)',
                },
                ticks: {
                  autoSkip: true,
                  maxTicksLimit: 10, // Limit the number of ticks to show
                },
              },
              y: {
                display: true,
              },
            },
          },
        });
      }
    }
  };

  // Null check for selectedData
  const durationInMinutes = selectedData ? Math.floor(selectedData.duration / 60) : 0;
  const durationInSeconds = selectedData ? selectedData.duration % 60 : 0;

  // Return the JSX structure for the component
  return (
    <div className="dashboard-container">
      <div className="dropdown-container">
        <label htmlFor="dropdown">Select a date:</label>
        <select
          id="dropdown"
          value={selectedOption}
          onChange={handleDropdownChange}
          className="custom-dropdown"
        >
          {options.map((option, index) => (
            <option key={index} value={option}>
              {option}
            </option>
          ))}
        </select>
      </div>

      <div className='display-container'>

        {/* Map container with added border */}
        <div id="map"></div>

        {/* Dashboard container */}
        <div className="dashboard">
          <h2>{selectedOption}</h2>
          <p><strong>Duration:</strong> {selectedData ? `${durationInMinutes}:${durationInSeconds < 10 ? '0' : ''}${durationInSeconds} min` : 'N/A'}</p>
          <p><strong>Distance:</strong> {selectedData ? `${(selectedData.distance / 1000).toFixed(3)} km` : 'N/A'}</p>
          <p><strong>Average Speed:</strong> {selectedData ? `${selectedData.avg_speed} km/h` : 'N/A'}</p>
          <p><strong>Altitude Difference:</strong> {selectedData ? `${selectedData.altitude_diff} m` : 'N/A'}</p>
          <div>
            <canvas id="altitudesChart"></canvas>
          </div>
        </div>

      </div>
    </div>
  );
};

export default MyDashboardPage;
