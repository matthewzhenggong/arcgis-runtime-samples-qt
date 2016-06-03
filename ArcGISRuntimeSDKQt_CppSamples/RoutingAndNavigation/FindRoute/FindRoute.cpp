// [WriteFile Name=FindRoute, Category=RoutingAndNavigation]
// [Legal]
// Copyright 2016 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

#include "FindRoute.h"

#include "Map.h"
#include "MapQuickView.h"
#include "Basemap.h"
#include "ArcGISVectorTiledLayer.h"
#include "GraphicsOverlay.h"
#include "Viewpoint.h"
#include "Point.h"
#include "SpatialReference.h"
#include "SimpleRenderer.h"
#include "SimpleLineSymbol.h"
#include "PictureMarkerSymbol.h"
#include "RouteTask.h"
#include "RouteParameters.h"
#include "Stop.h"

using namespace Esri::ArcGISRuntime;

FindRoute::FindRoute(QQuickItem* parent) :
    QQuickItem(parent),
    m_map(nullptr),
    m_mapView(nullptr),
    m_routeGraphicsOverlay(nullptr),
    m_stopsGraphicsOverlay(nullptr),
    m_routeTask(nullptr),
    m_routeParameters(),
    m_directions(nullptr)
{
}

FindRoute::~FindRoute()
{
}

void FindRoute::componentComplete()
{
    QQuickItem::componentComplete();

    // find QML MapView component
    m_mapView = findChild<MapQuickView*>("mapView");

    // create a new basemap instance
    auto navigationLayer = new ArcGISVectorTiledLayer(QUrl("http://www.arcgis.com/home/item.html?id=dcbbba0edf094eaa81af19298b9c6247"), this);
    auto basemap = new Basemap(navigationLayer, this);

    // create a new map instance
    m_map = new Map(basemap, this);
    m_map->setInitialViewpoint(Viewpoint(Point(-13041154, 3858170, SpatialReference(3857)), 1e5));

    // set map on the map view
    m_mapView->setMap(m_map);

    // create initial graphics overlays
    m_routeGraphicsOverlay = new GraphicsOverlay(this);
    auto simpleLineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor("cyan"), 4, this);
    auto simpleRenderer = new SimpleRenderer(simpleLineSymbol, this);
    m_routeGraphicsOverlay->setRenderer(simpleRenderer);
    m_stopsGraphicsOverlay = new GraphicsOverlay(this);
    m_mapView->graphicsOverlays()->append(m_routeGraphicsOverlay);
    m_mapView->graphicsOverlays()->append(m_stopsGraphicsOverlay);

    // connect to loadStatusChanged signal
    connect(m_map, &Map::loadStatusChanged, [this](LoadStatus loadStatus)
    {
        if (loadStatus == LoadStatus::Loaded)
        {
            addStopGraphics();
            setupRouteTask();
        }
    });
}

void FindRoute::addStopGraphics()
{
    // create the stop graphics' geometry
    Point stop1Geometry(-13041171, 3860988, SpatialReference(3857));
    Point stop2Geometry(-13041693, 3856006, SpatialReference(3857));

    // create the stop graphics' symbols
    auto stop1Symbol = getPictureMarkerSymbol(QUrl("qrc:/Samples/Routing and Navigation/FindRoute/pinA.png"));
    auto stop2Symbol = getPictureMarkerSymbol(QUrl("qrc:/Samples/Routing and Navigation/FindRoute/pinB.png"));

    // create the stop graphics
    auto stop1Graphic = new Graphic(stop1Geometry, stop1Symbol, this);
    auto stop2Graphic = new Graphic(stop2Geometry, stop2Symbol, this);

    // add to the overlay
    m_stopsGraphicsOverlay->graphics()->append(stop1Graphic);
    m_stopsGraphicsOverlay->graphics()->append(stop2Graphic);
}

// Helper function for creating picture marker symbols
PictureMarkerSymbol* FindRoute::getPictureMarkerSymbol(QUrl imageUrl)
{
    auto pictureMarkerSymbol = new PictureMarkerSymbol(imageUrl, this);
    pictureMarkerSymbol->setWidth(32);
    pictureMarkerSymbol->setHeight(32);
    pictureMarkerSymbol->setOffsetY(16);
    return pictureMarkerSymbol;
}

void FindRoute::setupRouteTask()
{
    // create the route task pointing to an online service
    m_routeTask = new RouteTask(QUrl("http://sampleserver6.arcgisonline.com/arcgis/rest/services/NetworkAnalysis/SanDiego/NAServer/Route"), this);

    // connect to loadStatusChanged signal
    connect(m_routeTask, &RouteTask::loadStatusChanged, [this](LoadStatus loadStatus)
    {
        if (loadStatus == LoadStatus::Loaded)
        {
            // Request default parameters once the task is loaded
            m_routeTask->generateDefaultParameters();
        }
    });

    // connect to generateDefaultParametersCompleted signal
    connect(m_routeTask, &RouteTask::generateDefaultParametersCompleted, [this](QUuid, RouteParameters routeParameters)
    {
        // Store the resulting route parameters
        m_routeParameters = routeParameters;
    });

    // connect to solveRouteCompleted signal
    connect(m_routeTask, &RouteTask::solveRouteCompleted, [this](QUuid, RouteResult routeResult)
    {
        // Add the route graphic once the solve completes
        auto generatedRoute = routeResult.routes().at(0);
        auto routeGraphic = new Graphic(generatedRoute.routeGeometry());
        m_routeGraphicsOverlay->graphics()->append(routeGraphic);

        // set the direction maneuver list model
        m_directions = generatedRoute.directionManeuvers(this);
        emit directionsChanged();

        // emit that the route has solved successfully
        emit solveRouteComplete();
    });

    // load the route task
    m_routeTask->load();
}

DirectionManeuverListModel* FindRoute::directions()
{
    return m_directions;
}

void FindRoute::solveRoute()
{
    if (!m_routeParameters.isEmpty())
    {
        // set parameters to return directions
        m_routeParameters.setReturnDirections(true);

        // clear previous route graphics
        m_routeGraphicsOverlay->graphics()->clear();

        // clear previous stops from the parameters
        m_routeParameters.clearStops();

        // set the stops to the parameters
        Stop stop1(m_stopsGraphicsOverlay->graphics()->at(0)->geometry());
        stop1.setName("Origin");
        Stop stop2(m_stopsGraphicsOverlay->graphics()->at(1)->geometry());
        stop2.setName("Destination");
        m_routeParameters.setStops(QList<Stop>() << stop1 << stop2);

        // solve the route with the parameters
        m_routeTask->solveRoute(m_routeParameters);
    }
}